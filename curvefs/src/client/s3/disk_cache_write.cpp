/*
 *  Copyright (c) 2020 NetEase Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * Project: curve
 * Created Date: 21-08-13
 * Author: hzwuhongsong
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#include "curvefs/src/client/s3/disk_cache_write.h"

namespace curvefs {

namespace client {

void DiskCacheWrite::Init(S3Client *client,
       std::shared_ptr<PosixWrapper> posixWrapper, const std::string cacheDir) {
    client_ = client;
    posixWrapper_ = posixWrapper;
    DiskCacheBase::Init(posixWrapper, cacheDir);
}

void DiskCacheWrite::AsyncUploadEnqueue(const std::string objName) {
    std::lock_guard<bthread::Mutex> lk(mtx_);
    waitUpload_.push_back(objName);
    cond_.notify_one();
}

int DiskCacheWrite::UploadFile(const std::string name) {
    std::string fileFullPath;
    bool fileExist;
    fileFullPath = GetCacheIoFullDir() + "/" + name;
    fileExist = IsFileExist(fileFullPath);
    if (!fileExist) {
        LOG(ERROR) << "file is not exist, file = " << name;
        return -1;
    }
    struct stat statFile;
    int fd, ret;
    ret = posixWrapper_->stat(fileFullPath.c_str(), &statFile);
    if (ret < 0) {
        LOG(ERROR) << "get file size error, file = " << name;
        return -1;
    }
    off_t fileSize = statFile.st_size;
    fd = posixWrapper_->open(fileFullPath.c_str(), O_RDONLY);
    if (fd < 0) {
        LOG(ERROR) << "open disk file error. errno = " << errno
                   << ", file = " << name;
        return fd;
    }

    uint64_t allocSize;
    allocSize = fileSize * sizeof(char) + 1;
    char *buffer = reinterpret_cast<char*>(posixWrapper_->malloc(allocSize));
    if (buffer == NULL) {
        LOG(ERROR) << "malloc failed in UploadFile.";
        posixWrapper_->close(fd);
        return -1;
    }
    void* memRet = posixWrapper_->memset(buffer, '0', fileSize * sizeof(char));
    if (memRet == NULL) {
        LOG(ERROR) << "memset failed in UploadFile.";
        posixWrapper_->free(buffer);
        posixWrapper_->close(fd);
        return -1;
    }
    ssize_t readLen = posixWrapper_->read(fd, buffer, fileSize);
    if (readLen < 0) {
        LOG(ERROR) << "read file error, ret = " << readLen
                   << ", errno = " << errno
                   << ", file = " << name;
        posixWrapper_->free(buffer);
        posixWrapper_->close(fd);
        return readLen;
    }
    if (readLen < fileSize) {
        LOG(ERROR) << "read disk file is not entirely. read len = " << readLen
                   << ", but file size = " << fileSize
                   << ", file = " << name;
        posixWrapper_->free(buffer);
        posixWrapper_->close(fd);
        return -1;
    }
    ret = client_->Upload(name, buffer, fileSize);
    if (ret < 0) {
        LOG(ERROR) << "upload object fail. object: " << name;
        posixWrapper_->free(buffer);
        posixWrapper_->close(fd);
        return -1;
    }
    posixWrapper_->free(buffer);
    posixWrapper_->close(fd);
    LOG(INFO) << "async upload file success, file = " << name;
    return 0;
}

int DiskCacheWrite::AsyncUploadFunc() {
    std::list<std::string> toUpload;
    std::string fileFullPath;
    fileFullPath = GetCacheIoFullDir();
    bool ret = IsFileExist(fileFullPath);
    if (!ret) {
        LOG(ERROR) << "cache write dir is not exist.";
        return -1;
    }
    LOG(INFO) << "async upload function start.";
    while (true) {
        if (!isRunning_) {
            LOG(INFO) << "async upload thread stop.";
            return 0;
        }
        toUpload.clear();
        {
            std::unique_lock<bthread::Mutex> lk(mtx_);
            while (waitUpload_.empty()) {
                cond_.wait(lk);
            }
            toUpload.swap(waitUpload_);
        }
        LOG(INFO) << "async upload file size = " << toUpload.size();
        std::list<std::string>::iterator iter;
        int ret;
        for (iter = toUpload.begin(); iter != toUpload.end(); iter++) {
            ret = UploadAndRemove(*iter);
            if (ret < 0) {
                LOG(ERROR) << "upload and remove file fail, file = "
                           << *iter;
                continue;
            }
        }
    }
    return 0;
}

int DiskCacheWrite::AsyncUploadRun() {
    if (isRunning_.exchange(true)) {
        LOG(INFO) << "AsyncUpload thread is on running.";
        return -1;
    }
    LOG(INFO) << "AsyncUpload thread is on running.";
    backEndThread_ = std::thread(
        &DiskCacheWrite::AsyncUploadFunc, this);
    return 0;
}

int DiskCacheWrite::AsyncUploadStop() {
    if (isRunning_.exchange(false)) {
        LOG(INFO) << "stop AsyncUpload thread...";
        backEndThread_.join();
        LOG(INFO) << "stop AsyncUpload thread ok.";
        return -1;
    } else {
        LOG(INFO) << "AsyncUpload thread not running.";
    }
    return 0;
}

int DiskCacheWrite::UploadAllCacheWriteFile() {
    LOG(INFO) << "upload all cached write file start.";
    std::string fileFullPath;
    bool ret;
    DIR *cacheWriteDir = NULL;
    struct dirent *cacheWriteDirent = NULL;
    fileFullPath = GetCacheIoFullDir();
    ret = IsFileExist(fileFullPath);
    if (!ret) {
        LOG(ERROR) << "cache write dir is not exist.";
        return -1;
    }
    cacheWriteDir = posixWrapper_->opendir(fileFullPath.c_str());
    if (!cacheWriteDir) {
        LOG(ERROR) << "opendir error， errno = " << errno;
        return -1;
    }
    int doRet;
    while ((cacheWriteDirent = posixWrapper_->readdir(cacheWriteDir)) != NULL) {
        if ((!strncmp(cacheWriteDirent->d_name, ".", 1))
             || (!strncmp(cacheWriteDirent->d_name, "..", 2)))
            continue;

        std::string fileName = cacheWriteDirent->d_name;
        doRet = UploadAndRemove(fileName);
        if (doRet < 0) {
            LOG(ERROR) << "upload and remove file fail, file = "
                       << fileName;
            continue;
        }
    }
    doRet = posixWrapper_->closedir(cacheWriteDir);
    if (doRet < 0) {
        LOG(ERROR) << "opendir error， errno = " << errno;
        return doRet;
    }
    LOG(INFO) << "upload all cached write file end.";
    return 0;
}

int DiskCacheWrite::UploadAndRemove(const std::string fileName) {
    // upload file to S3
    int ret = UploadFile(fileName);
    if (ret < 0) {
        LOG(ERROR) << "async upload file fail, file = " << fileName;
        return -1;
    }
    // del disk file
    std::string fileFullPath;
    fileFullPath = GetCacheIoFullDir();
    std::string fullFileName = fileFullPath + "/" + fileName;
    ret = posixWrapper_->remove(fullFileName.c_str());
    if (ret < 0) {
        LOG(ERROR) << "remove disk file error, file = " << fileName
                   << ", errno = " << errno;
        return -1;
    }
    LOG(INFO) << "upload and remove file success, file = "
              << fileName;
    return 0;
}

int DiskCacheWrite::WriteDiskFile(const std::string fileName,
                      const char* buf, uint64_t length, bool force) {
    LOG(INFO) << "WriteDiskFile start. name = " << fileName
                 << ", force = " << force
                 << ", length = " << length;
    std::string fileFullPath;
    int fd, ret;
    fileFullPath = GetCacheIoFullDir() + "/" + fileName;
    fd = posixWrapper_->open(fileFullPath.c_str(), O_RDWR|O_CREAT);
    if (fd < 0) {
        LOG(ERROR) << "open disk file error. errno = " << errno
                   << ", file = " << fileName;
        return fd;
    }
    ssize_t writeLen = posixWrapper_->write(fd, buf, length);
    if (writeLen < length) {
        LOG(ERROR) << "write disk file error. ret = " << writeLen
                   << ", file = " << fileName;
        posixWrapper_->close(fd);
        return -1;
    }
    // force to flush
    if (force) {
        ret = posixWrapper_->fdatasync(fd);
        if (ret < 0) {
            LOG(ERROR) << "fdatasync error. errno = " << errno
                       << ", file = " << fileName;
            posixWrapper_->close(fd);
            return -1;
        }
    }
    ret = posixWrapper_->close(fd);
    if (ret < 0) {
        LOG(ERROR) << "close disk file error. errno = " << errno
                   << ", file = " << fileName;
        return -1;
    }

    LOG(INFO) << "WriteDiskFile success. name = " << fileName
              << ", force = " << force
              << ", length = " << length;
    return writeLen;
}

}  // namespace client
}  // namespace curvefs
