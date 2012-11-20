package main.conn.udp;

import main.conn.UDPConnection;
import main.data.Frame;
import main.utils.Pipeline;
import org.apache.log4j.Logger;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.concurrent.atomic.AtomicBoolean;

public class UDPReceiver implements Runnable {

    private static final Logger logger = Logger.getLogger(UDPReceiver.class);

    private final DatagramSocket socket;

    private final AtomicBoolean running;

    private final Pipeline<Frame> receiverQueue;

    private final UDPConnection udpConnection;

    private Thread second;

    public UDPReceiver(DatagramSocket socket, AtomicBoolean running, UDPConnection udpConnection,
                       Pipeline<Frame> receiverQueue) {
        this.socket = socket;
        this.running = running;
        this.udpConnection = udpConnection;
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
        byte[] data = new byte[1024];
        DatagramPacket packet = new DatagramPacket(data, data.length);

        logger.info("Starting receiver thread");
        try {
            while (isRunning()) {
                socket.receive(packet);

                byte[] stream = new byte[packet.getLength()];
                System.arraycopy(packet.getData(), 0, stream, 0, packet.getLength());

                logger.info("From sensor: " + packet.getAddress().toString() + ":" + packet.getPort());
                logger.debug("Data: " + new String(stream).replaceAll("\\n", "^]"));

                receiverQueue.add(new Frame(stream, packet.getAddress(), packet.getPort()));
            }
        } catch (Exception e) {
            logger.error("Error during receiving frame", e);
            setRunning(false);
            if (second != null) {
                second.interrupt();
            }
        }
        logger.info("Stopping receiver thread");
        udpConnection.close();
    }
}
