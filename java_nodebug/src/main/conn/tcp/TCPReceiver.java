package main.conn.tcp;

import main.conn.TCPConnection;
import main.data.Frame;
import main.utils.Pipeline;

import java.io.InputStream;
import java.net.InetAddress;
import java.util.concurrent.atomic.AtomicBoolean;

public class TCPReceiver implements Runnable {

    private final InputStream in;

    private final AtomicBoolean running;

    private final Pipeline<Frame> receiverQueue;

    private final TCPConnection tcpConnection;

    private final int udpPort;

    private final InetAddress udpAddress;

    private Thread second;

    public TCPReceiver(InputStream in, AtomicBoolean running, TCPConnection tcpConnection,
                       int udpPort, InetAddress udpAddress, Pipeline<Frame> receiverQueue) {
        this.in = in;
        this.running = running;
        this.tcpConnection = tcpConnection;
        this.udpPort = udpPort;
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

        try {
            while (isRunning()) {
                length = in.read(data);
                tmp = new byte[length];
                System.arraycopy(data, 0, tmp, 0, length);

                receiverQueue.add(new Frame(tmp, udpAddress, udpPort));
            }
        } catch (Exception e) {
            setRunning(false);
            if (second != null) {
                second.interrupt();
            }
        }
        tcpConnection.close();
    }

}