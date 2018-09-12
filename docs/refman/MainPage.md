Open Enclave SDK
================

Introduction
------------

Open Enclave (OE) is an SDK for building enclave applications in C and C++. An
enclave application partitions itself into two components (1) An untrusted component (called the
host) and (2) A trusted component (called the enclave). An enclave is a secure
container whose memory is protected from entities outside the enclave. These
protections allow enclaves to perform secure computations with assurances that
secrets will not be compromised.

This SDK is a fully open-source and transparent project, which plans to generalize enclave application models across 
enclave implementations from different hardware vendors. It's a non-vendor specific solution that supports enclave applications both on
Linux and Windows platforms.

The current implementation of Open Enclave is built on [Intel Software Guard Extensions (SGX)](https://software.intel.com/en-us/sgx), other enclave architectures (such as solutions from AMD or ARM) will be added in the future. This public preview focuses on the Linux platform.

Design Overview
-------------

The [Design Overview](https://github.com/Microsoft/openenclave/blob/master/docs/DesignOverview.pdf) document provides a brief design overview of the Open Enclave SDK. It describes the parts of the SDK and how they work together to create, invoke, and terminate enclaves. 

Git repository
---------------

To get started on developing Open Enclave applications and contributing to Open Enclave SDK, visit the [Open Enclave repo](https://github.com/Microsoft/openenclave)

Licensing
=========

Microsoft plans to release the [Open Enclave SDK under the MIT license](https://github.com/Microsoft/openenclave/blob/master/LICENSE)
