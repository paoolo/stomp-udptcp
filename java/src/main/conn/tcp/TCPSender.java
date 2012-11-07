package main.conn.tcp;

import main.conn.TCPConnection;
import main.data.Frame;
import main.utils.Pipeline;
import org.apache.log4j.Logger;

import java.io.OutputStream;
import java.util.concurrent.atomic.AtomicBoolean;

public class TCPSender implements Runnable {

    private static final Logger logger = Logger.getLogger(TCPSender.class);

    private final OutputStream out;

    private final AtomicBoolean running;

    private final Pipeline<Frame> senderQueue;

    private final TCPConnection tcpConnection;

    private Thread second;

    public TCPSender(OutputStream out, AtomicBoolean running, TCPConnection tcpConnection,
                     Pipeline<Frame> senderQueue) {
        this.out = out;
        this.running = running;
        this.tcpConnection = tcpConnection;
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
        Frame frame;

        logger.info("Starting sender thread");
        try {
            while (isRunning()) {
                frame = senderQueue.remove();
                out.write(frame.data);

                logger.debug("Sending data to broker: " + new String(frame.data));
            }
        } catch (Exception e) {
            logger.error("Error during sending frame", e);
            setRunning(false);
            if (second != null) {
                second.interrupt();
            }
        }
        logger.info("Stopping sender thread");
        tcpConnection.close();
    }

}