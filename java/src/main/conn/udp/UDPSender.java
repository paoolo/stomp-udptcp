package main.conn.udp;

import main.conn.UDPConnection;
import main.data.Frame;
import main.utils.Pipeline;
import org.apache.log4j.Logger;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.concurrent.atomic.AtomicBoolean;

public class UDPSender implements Runnable {

    private static final int CRC_OFFSET = 8;

    private static final Logger logger = Logger.getLogger(UDPSender.class);

    private final DatagramSocket socket;

    private final AtomicBoolean running;

    private final Pipeline<Frame> senderQueue;

    private final UDPConnection udpConnection;

    private Thread second;

    public UDPSender(DatagramSocket socket, AtomicBoolean running, UDPConnection udpConnection,
                     Pipeline<Frame> senderQueue) {
        this.socket = socket;
        this.running = running;
        this.udpConnection = udpConnection;
        this.senderQueue = senderQueue;
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
        DatagramPacket packet;
        Frame frame;

        logger.info("Starting sender thread");
        try {
            while (isRunning()) {
                frame = senderQueue.remove();
                packet = new DatagramPacket(frame.data, frame.data.length, frame.address, frame.port);
                socket.send(packet);

                logger.info("To sensor: " + frame.address + ":" + frame.port);
                logger.debug("Data: " + new String(frame.data).replaceAll("\\n", "^]"));
            }
        } catch (Exception e) {
            logger.error("Error during sending frame", e);
            setRunning(false);
            if (second != null) {
                second.interrupt();
            }
        }
        logger.info("Stopping sender thread");
        udpConnection.close();
    }
}