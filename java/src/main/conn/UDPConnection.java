package main.conn;

import main.conn.udp.UDPReceiver;
import main.conn.udp.UDPSender;
import main.data.Frame;
import main.utils.Pipeline;
import main.utils.pipes.SinglePipeline;
import org.apache.log4j.Logger;

import java.net.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class UDPConnection {

    private static final Logger logger = Logger.getLogger(UDPConnection.class);

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
        logger.info("Bound to " + udpAddress + ":" + udpPort);

        UDPReceiver receiver = new UDPReceiver(datagramSocket, running, this,
                receiverQueue);
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
                logger.info("Closed");
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
