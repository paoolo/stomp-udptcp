package main.conn.tcp;

import main.conn.TCPConnection;
import main.data.Frame;
import main.utils.Pipeline;
import org.apache.log4j.Logger;

import java.io.InputStream;
import java.net.InetAddress;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;

public class TCPReceiver implements Runnable {

    private static final Logger logger = Logger.getLogger(TCPReceiver.class);

    private final InputStream in;

    private final AtomicBoolean running;

    private final int udpPort;

    private final TCPConnection tcpConnection;

    private final InetAddress udpAddress;

    private final Pipeline<Frame> receiverQueue;

    private Thread second;

    public TCPReceiver(InputStream in, AtomicBoolean running, TCPConnection tcpConnection,
                       int udpPort, InetAddress udpAddress, Pipeline<Frame> receiverQueue) {
        this.in = in;
        this.running = running;
        this.udpPort = udpPort;
        this.tcpConnection = tcpConnection;
        this.udpAddress = udpAddress;
        this.receiverQueue = receiverQueue;
    }

    private boolean isRunning() {
        synchronized (running) {
            return running.get();
        }
    }

    private void setRunning(boolean running) {
        synchronized (this.running) {
            this.running.set(running);
        }
    }

    public void setSecond(Thread second) {
        if (second != null) {
            synchronized (this) {
                if (this.second == null) {
                    this.second = second;
                }
            }
        }
    }

    @Override
    public void run() {
        int length;
        byte[] tmp, data;

        data = new byte[1024];

        logger.info("Starting receiver thread");
        try {
            while (isRunning()) {
                length = in.read(data);
                tmp = new byte[length];
                System.arraycopy(data, 0, tmp, 0, length);

                logger.debug("Receiving data from broker: " + Arrays.toString(tmp));
                receiverQueue.add(new Frame(tmp, udpAddress, udpPort));
            }
        } catch (Exception e) {
            logger.error("Error during receiving frame", e);
            setRunning(false);
            if (second != null) {
                second.interrupt();
            }
        }
        logger.info("Stopping receiver thread");
        tcpConnection.close();
    }

}