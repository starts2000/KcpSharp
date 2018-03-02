#include "KcpTest.h"
#include "KcpSharp.h"

Starts2000::KcpSharp::KcpTest::KcpTest() : KcpTest(10, 60, 125, 1000)
{
}

Starts2000::KcpSharp::KcpTest::KcpTest(int lostrate, int rttmi, int rttmax, int nmax)
{
	this->lostrate = lostrate;
	this->rttmi = rttmi;
	this->rttmax = rttmax;
	this->nmax = nmax;
}

int Starts2000::KcpSharp::KcpTest::UdpOutput(IntPtr buffer, int len, IntPtr kcp, IntPtr user) {
	auto userToken = GCHandle::FromIntPtr(user).Target;
	vnet->send((int)userToken, buffer.ToPointer(), len);
	return 0;
}

void Starts2000::KcpSharp::KcpTest::Test(int mode)
{
	// 创建模拟网络：丢包率10 % ，Rtt 60ms~125ms
	this->vnet = new LatencySimulator(lostrate, rttmi, rttmax, nmax);

	// 创建两个端点的 kcp对象，第一个参数 conv是会话编号，同一个会话需要相同
	// 最后一个是 user参数，用来传递标识
	auto kcp1 = gcnew Kcp(0x11223344, (Object ^)0);
	auto kcp2 = gcnew Kcp(0x11223344, (Object ^)1);

	auto output = gcnew KcpOutputHandler(this, &KcpTest::UdpOutput);

	// 设置kcp的下层输出，这里为 udp_output，模拟udp网络输出函数
	kcp1->SetOutput(output);
	kcp2->SetOutput(output);

	IUINT32 current = iclock();
	IUINT32 slap = current + 20;
	IUINT32 index = 0;
	IUINT32 next = 0;
	IINT64 sumrtt = 0;
	int count = 0;
	int maxrtt = 0;

	// 配置窗口大小：平均延迟200ms，每20ms发送一个包，
	// 而考虑到丢包重发，设置最大收发窗口为128
	kcp1->SetWndSize(128, 128);
	kcp2->SetWndSize(128, 128);

	// 判断测试用例的模式
	if (mode == 0) {
		// 默认模式
		kcp1->Nodelay(0, 10, 0, 0);
		kcp2->Nodelay(0, 10, 0, 0);
	}
	else if (mode == 1) {
		// 普通模式，关闭流控等
		kcp1->Nodelay(0, 10, 0, 1);
		kcp2->Nodelay(0, 10, 0, 1);
	}
	else {
		// 启动快速模式
		// 第二个参数 nodelay-启用以后若干常规加速将启动
		// 第三个参数 interval为内部处理时钟，默认设置为 10ms
		// 第四个参数 resend为快速重传指标，设置为2
		// 第五个参数 为是否禁用常规流控，这里禁止
		kcp1->Nodelay(0, 10, 2, 1);
		kcp2->Nodelay(0, 10, 2, 1);

		kcp1->IKcp->rx_minrto = 10;
		kcp1->IKcp->fastresend = 1;
	}

	array<Byte>^ buffer = gcnew array<Byte>(2000);
	pin_ptr<void> pBuffer = &buffer[0];
	int hr;

	IUINT32 ts1 = iclock();

	while (1) {
		isleep(1);
		current = iclock();
		kcp1->Update(iclock());
		kcp2->Update(iclock());

		// 每隔 20ms，kcp1发送数据
		for (; current >= slap; slap += 20) {
			auto pBuf = static_cast<IUINT32 *>(pBuffer);
			pBuf[0] = index++;
			pBuf[1] = current;
			
			kcp1->Send(buffer, 8);
		}

		// 处理虚拟网络：检测是否有udp包从p1->p2
		while (1) {
			hr = vnet->recv(1, pBuffer, 2000);
			if (hr < 0) break;
			// 如果 p2收到udp，则作为下层协议输入到kcp2
			kcp2->Input(buffer, hr);
		}

		// 处理虚拟网络：检测是否有udp包从p2->p1
		while (1) {
			hr = vnet->recv(0, pBuffer, 2000);
			if (hr < 0) break;
			// 如果 p1收到udp，则作为下层协议输入到kcp1
			kcp1->Input(buffer, hr);
		}

		// kcp2接收到任何包都返回回去
		while (1) {
			hr = kcp2->Recv(buffer, 10);
			// 没有收到包就退出
			if (hr < 0) break;
			// 如果收到包就回射
			kcp2->Send(buffer, hr);
		}

		// kcp1收到kcp2的回射数据
		while (1) {
			hr = kcp1->Recv(buffer, 10);
			// 没有收到包就退出
			if (hr < 0) break;

			auto pBuf = static_cast<char *>(pBuffer);
			IUINT32 sn = *(IUINT32*)(pBuf + 0);
			IUINT32 ts = *(IUINT32*)(pBuf + 4);
			IUINT32 rtt = current - ts;

			if (sn != next) {
				// 如果收到的包不连续
				printf("ERROR sn %d<->%d\n", (int)count, (int)next);
				return;
			}

			next++;
			sumrtt += rtt;
			count++;
			if (rtt > (IUINT32)maxrtt) maxrtt = rtt;

			printf("[RECV] mode=%d sn=%d rtt=%d\n", mode, (int)sn, (int)rtt);
		}
		if (next > 1000) break;
	}

	ts1 = iclock() - ts1;

	delete kcp1;
	delete kcp2;

	const char *names[3] = { "default", "normal", "fast" };
	printf("dotnet wrapper KCP test.\n");
	printf("%s mode result (%dms):\n", names[mode], (int)ts1);
	printf("avgrtt=%d maxrtt=%d tx=%d\n", (int)(sumrtt / count), (int)maxrtt, (int)vnet->tx1);
	printf("press enter to next ...\n");
	delete this->vnet;
	char ch; scanf("%c", &ch);
}

Starts2000::KcpSharp::KcpTest::~KcpTest()
{
}

Starts2000::KcpSharp::KcpTest::!KcpTest()
{
	if (this->vnet) {
		delete this->vnet;
	}
}
