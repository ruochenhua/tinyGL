# 关于tinyGL

这个工程是我用于学习和实践引擎的各个方面技术的项目。从基础的OpenGL接入开始，搭起一个能够实现各种效果的框架。

目前包括以下几个部分：
- 渲染：tiny render，渲染能力。目前利用OpenGL实现光栅化渲染；后续考虑接入类似于DXR的光线追踪框架能力。
- 物理：tap engine，物理模拟能力。自己编写实现，作为组建提供真是的物理效果。

场景的描述目前计划是基于yaml实现。
目前计划是尽量实现支持多平台（后续接入DXR的话这个feature就只在windows上支持）