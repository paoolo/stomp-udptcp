package main.conn.udp;

import main.conn.UDPConnection;
import main.data.Frame;
import main.utils.Pipeline;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.concurrent.atomic.AtomicBoolean;

public class UDPSender implements Runnable {

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

        try {
            while (isRunning()) {
                frame = senderQueue.remove();
                packet = new DatagramPacket(frame.data, frame.data.length, frame.address, frame.port);
                socket.send(packet);
            }
        } catch (Exception e) {
            setRunning(false);
            if (second != null) {
                second.interrupt();
            }
        }
        udpConnection.close();
    }
}