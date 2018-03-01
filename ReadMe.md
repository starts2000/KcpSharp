## KcpSharp ##
- [KCP](https://github.com/skywind3000/kcp "KCP") 是一个快速可靠协议，能以比 TCP 浪费 10%-20% 的带宽的代价，换取平均延迟降低  30%-40%，且最大延迟降低三倍的传输效果。

- KcpSharp 是对 [KCP](https://github.com/skywind3000/kcp "KCP") 完全的 .NET 封装，以便在 .NET 中方便的使用 [KCP](https://github.com/skywind3000/kcp "KCP")。KcpSharp 不仅对 KCP 进行了封装， 还对 KCP 的模拟测试方法也进行了封装。

## 解决方案说明 ##
- Starts2000.KcpSharp：KCP C++/CLI 封装。
- Starts2000.KcpSharp.Test：KcpSharp 模拟测试项目，运行后输入 'N' ，执行 KCP 自带的模拟测试；输入 'W' ,执行 Kcp .NET 模拟测试；