package main.conn;

import main.conn.tcp.TCPReceiver;
import main.conn.tcp.TCPSender;
import main.data.Frame;
import main.utils.Pipeline;
import main.utils.pipes.SinglePipeline;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.concurrent.atomic.AtomicBoolean;

public class TCPConnection {

    private static final Logger logger = Logger.getLogger(TCPConnection.class);

    private final Thread receiverThread, senderThread;

    private final AtomicBoolean running = new AtomicBoolean(false);

    private final Pipeline<Frame> senderQueue = new SinglePipeline<Frame>();

    private final Pipeline<Frame> receiverQueue;

    private final Object closing = new Object();

    private final Socket socket;

    private void setRunning(boolean running) {
        synchronized (this.running) {
            this.running.set(running);
        }
    }

    public boolean isRunning() {
        synchronized (running) {
            return running.get();
        }
    }

    public Pipeline<Frame> getReceiverQueue() {
        return receiverQueue;
    }

    public Pipeline<Frame> getSenderQueue() {
        return senderQueue;
    }

    public TCPConnection(InetAddress tcpAddress, int tcpPort,
                         InetAddress udpAddress, int udpPort,
                         Pipeline<Frame> receiverQueue) throws Exception {

        this.receiverQueue = receiverQueue;

        socket = new Socket(tcpAddress, tcpPort);
        logger.info("Connected to\n" + tcpAddress + ":" + tcpPort);

        TCPReceiver receiver = new TCPReceiver(socket.getInputStream(), running, this,
                udpPort, udpAddress, this.receiverQueue);
        TCPSender sender = new TCPSender(socket.getOutputStream(), running, this,
                this.senderQueue);

        receiverThread = new Thread(receiver, "tcp-receiver");
        senderThread = new Thread(sender, "tcp-sender");

        receiver.setSecond(senderThread);
        sender.setSecond(receiverThread);

        setRunning(true);
        receiverThread.start();
        senderThread.start();
    }

    public void close() {
        synchronized (closing) {
            if (socket.isConnected() && !socket.isClosed()) {
                try {
                    socket.close();
                    logger.info("Closed");
                } catch (IOException e) {
                    logger.warn("Error during closing socket", e);
                }
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