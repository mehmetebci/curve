[中文版](README_cn.md)


<img src="docs/images/curve-logo1.png"/>

# CURVE

[![Docs](https://img.shields.io/badge/docs-latest-green.svg)](https://github.com/opencurve/curve/tree/master/docs)
[![Releases](https://img.shields.io/github/v/release/opencurve/curve?include_prereleases)](https://github.com/opencurve/curve/releases)
[![LICENSE](https://img.shields.io/badge/licence-Apache--2.0%2FGPL-blue)](https://github.com/opencurve/curve/blob/master/LICENSE)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/6136/badge)](https://bestpractices.coreinfrastructure.org/projects/6136)


Curve is a high-performance, lightweight-operation, cloud-native open source distributed storage system. Can be applied to mainstream cloud-native infrastructure platforms: connect to OpenStack platform to provide high-performance block storage services for cloud VM; connect to Kubernetes to provide RWO, RWX and other types of persistent volumes; connect to PolarFS as a high-performance storage for cloud-native databases, perfectly supports the storage-computing separation architecture of cloud-native databases. Curve can also be used as a cloud storage middleware using S3-compatible object storage as a data storage engine, providing cost-effective shared file storage for public cloud users.

Curve has hosted by the Cloud Native Computing Foundation (CNCF) as a sandbox project.

## Curve Architecture
The architecture overview of Curve is as follows:

<image src="docs/images/Curve-arch.png" width=70%>

Curve supports deployment in private cloud and public cloud environment, and can also be used in hybrid cloud. The deployment architecture in private cloud environment is as follows:

<image src="docs/images/Curve-deploy-on-premises-idc.png" width=60%>

The CurveFS shared file storage system can be elastically scaled to public cloud storage, which can provide users with greater capacity elasticity, lower costs, and better performance experience:

<image src="docs/images/Curve-deploy-on-public-cloud.png" width=55%>

## Curve Block Service vs Ceph Block Device

Curve: v1.2.0

Ceph: L/N
### Performance
Curve random read and write performance far exceeds Ceph in the block storage scenario.

Environment：3 replicas on a 6-node cluster, each node has 20xSATA SSD, 2xE5-2660 v4 and 256GB memory.

Single Vol：
<image src="docs/images/1-nbd-en.png">

Multi Vols：
<image src="docs/images/10-nbd-en.png">

### Stability
The stability of the common abnormal Curve is better than that of Ceph in the block storage scenario.
| Fault Case | One Disk Failure | Slow Disk Detect | One Server Failure | Server Suspend Animation |
| :----: | :----: | :----: | :----: | :----: |
| Ceph | jitter 7s | Continuous io jitter | jitter 7s | unrecoverable |
| Curve | jitter 4s | no effect | jitter 4s | jitter 4s |
### Ops
Curve ops is more friendly than Curve in the block storage scenario.
| Ops scenarios | Upgrade clients | Balance |
| :----: | :----: | :----: |
| Ceph | do not support live upgrade | via plug-in with IO influence |
| Curve | support live upgrade with second jitter | auto with no influence on IO |

## Design Documentation

- Wanna have a glance at Curve? Click here for [Intro to Curve](https://www.opencurve.io/)!
- Want more details about CurveBS? Our documentation for every component:
  - [NEBD](docs/en/nebd_en.md)
  - [MDS](docs/en/mds_en.md)
  - [Chunkserver](docs/en/chunkserver_design_en.md)
  - [Snapshotcloneserver](docs/en/snapshotcloneserver_en.md)
  - [CURVE quality control](docs/en/quality_en.md)
  - [CURVE monitoring](docs/en/monitor_en.md)
  - [Client](docs/en/client_en.md)
  - [Client Python API](docs/en/curve-client-python-api_en.md)
- Application based on CurveBS
  - [Work with k8s](docs/en/k8s_csi_interface_en.md)
- Want more details about CurveFS? Our documentation for every component(Chinese version only, English version will be uploaded soon):
  - [Architecture design](docs/cn/curvefs_architecture.md)
  - [Client design](docs/cn/curvefs-client-design.md)
  - [Metadata management](docs/cn/curvefs-metaserver-overview.md)
  - [Data caching](https://github.com/opencurve/curve-meetup-slides/blob/main/CurveFS/Curve%E6%94%AF%E6%8C%81S3%20%E6%95%B0%E6%8D%AE%E7%BC%93%E5%AD%98%E6%96%B9%E6%A1%88.pdf)
  - [Space allocation](https://github.com/opencurve/curve-meetup-slides/blob/main/CurveFS/Curve%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F%E7%A9%BA%E9%97%B4%E5%88%86%E9%85%8D%E6%96%B9%E6%A1%88.pdf)
  - [more details](https://github.com/opencurve/curve-meetup-slides/tree/main/CurveFS)

## Quick Start of CurveBS

In order to improve the operation and maintenance convenience of Curve, we designed and developed the [CurveAdm](https://github.com/opencurve/curveadm) project, which is mainly used for deploying and managing Curve clusters. Currently, it supports the deployment of CurveBS & CurveFS (scaleout, upgrade and other functions are under development), please refer to the [CurveAdm User Manual](https://github.com/opencurve/curveadm/wiki) for related documentation, and install the CurveAdm tool according to the manual before deploying the Curve cluster.

### Deploy an all-in-one environment (to try how CURVE works)
Please refer to the CurveBS cluster deployment steps in the [CurveAdm User Manual](https://github.com/opencurve/curveadm/wiki/curvebs-cluster-deployment) , for the single-machine experience environment, please use the template about "cluster topology file for single-machine deployment".


[Deploy on single machine - deprecated method](docs/en/deploy_en.md#deploy-on-single-machine)

### Deploy multi-machine cluster (try it in production environment)
Please refer to the CurveBS cluster deployment steps in the [CurveAdm User Manual](https://github.com/opencurve/curveadm/wiki/curvebs-cluster-deployment) , please use the template about "cluster topology file for multi-machine deployment".


[Deploy on multiple machines - deprecated method](docs/en/deploy_en.md#deploy-on-multiple-machines)

### curve_ops_tool introduction

[curve_ops_tool introduction](docs/en/curve_ops_tool_en.md)

## Quick Start of CurveFS
In order to improve the operation and maintenance convenience of Curve, we designed and developed the [CurveAdm](https://github.com/opencurve/curveadm) project, which is mainly used for deploying and managing Curve clusters. Currently, it supports the deployment of CurveBS & CurveFS, please refer to the [CurveAdm User Manual](https://github.com/opencurve/curveadm/wiki) for related documentation, and install the CurveAdm tool according to the manual before deploying the Curve cluster.

Detail for deploying CurveFS cluster: [CurveFS ​​deployment](https://github.com/opencurve/curveadm/wiki/curvefs-cluster-deployment)

### curvefs_tool introduction

[curvefs_tool introduction](curvefs/src/tools#readme)


## For Developers

How to participate in the Curve project development is detailed in [Curve Community Guidelines](Community_Guidelines.md)

### Deploy build and development environment

[development environment deployment](docs/en/build_and_run_en.md)

### Compile test cases and run
[test cases compiling and running](docs/en/build_and_run_en.md#test-case-compilation-and-execution)

### FIO curve block storage engine
Fio curve engine is added, you can clone https://github.com/opencurve/fio and compile the fio tool with our engine(depend on nebd lib), fio command line example: `./fio --thread --rw=randwrite --bs=4k --ioengine=nebd --nebd=cbd:pool//pfstest_test_ --iodepth=10 --runtime=120 --numjobs=10 --time_based --group_reporting --name=curve-fio-test`

## Release Cycle
- CURVE release cycle：Half a year for major version, 1~2 months for minor version

- Versioning format: We use a sequence of three digits and a suffix (x.y.z{-suffix}), x is the major version, y is the minor version, and z is for bugfix. The suffix is for distinguishing beta (-beta), RC (-rc) and GA version (without any suffix). Major version x will increase 1 every half year, and y will increase every 1~2 months. After a version is released, number z will increase if there's any bugfix.

## Branch

All the developments will be done under master branch. If there's any new version to establish, a new branch release-x.y will be pulled from the master, and the new version will be released from this branch.

## Feedback & Contact

- [Github Issues](https://github.com/openCURVE/CURVE/issues)：You are sincerely welcomed to issue any bugs you came across or any suggestions through Github issues. If you have any question you can refer to our FAQ or join our user group for more details.
- [FAQ](https://github.com/openCURVE/CURVE/wiki/CURVE-FAQ)：Frequently asked question in our user group, and we'll keep working on it.
- User group：We use Wechat group currently.
- [Double Week Meetings](https://github.com/opencurve/curve-meetup-slides/tree/main/2022): We have an online community meeting every two weeks which talk about what Curve is doing and planning to do. The time and links of the meeting are public in the user group and [Double Week Meetings](https://github.com/opencurve/curve-meetup-slides/tree/main/2022).

<img src="docs/images/curve-wechat.jpeg" style="zoom: 75%;" />
