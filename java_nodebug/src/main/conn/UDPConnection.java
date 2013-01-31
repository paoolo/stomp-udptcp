package main.conn;

import main.conn.udp.UDPReceiver;
import main.conn.udp.UDPSender;
import main.data.Frame;
import main.utils.Pipeline;
import main.utils.pipes.SinglePipeline;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.concurrent.atomic.AtomicBoolean;

public class UDPConnection {

    private final Thread receiverThread, senderThread;

    private final AtomicBoolean running = new AtomicBoolean(false);

    private final Pipeline<Frame> senderQueue = new SinglePipeline<Frame>();

    private final Pipeline<Frame> receiverQueue = new SinglePipeline<Frame>();

    private final Object closing = new Object();

    private final DatagramSocket datagramSocket;

    private void setRunning(boolean running) {
        synchronized (this.running) {
            this.running.set(running);
        }
    }

    public Pipeline<Frame> getReceiverQueue() {
        return receiverQueue;
    }

    public Pipeline<Frame> getSenderQueue() {
        return senderQueue;
    }

    public UDPConnection(InetAddress udpAddress, int udpPort) throws Exception {

        datagramSocket = new DatagramSocket(udpPort, udpAddress);

        UDPReceiver receiver = new UDPReceiver(datagramSocket, running, this,
                receiverQueue, senderQueue);
        UDPSender sender = new UDPSender(datagramSocket, running, this,
                senderQueue);

        receiverThread = new Thread(receiver, "udp-receiver");
        senderThread = new Thread(sender, "udp-sender");

        receiver.setSecond(senderThread);
        sender.setSecond(receiverThread);

        setRunning(true);
        receiverThread.start();
        senderThread.start();
    }

    public void close() {
        synchronized (closing) {
            if (datagramSocket.isBound() && !datagramSocket.isClosed()) {
                datagramSocket.close();
            }
        }
    }

    public void stop() {
        setRunning(false);
        senderThread.interrupt();
        receiverThread.interrupt();
        close();
    }
}
