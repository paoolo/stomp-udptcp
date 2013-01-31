package main.conn.udp;

import main.conn.UDPConnection;
import main.data.Frame;
import main.utils.Pipeline;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.concurrent.atomic.AtomicBoolean;

public class UDPReceiver implements Runnable {

    private final DatagramSocket socket;

    private final AtomicBoolean running;

    private final Pipeline<Frame> receiverQueue;

    private final Pipeline<Frame> senderQueue;

    private final UDPConnection udpConnection;

    private Thread second;

    public UDPReceiver(DatagramSocket socket, AtomicBoolean running, UDPConnection udpConnection,
                       Pipeline<Frame> receiverQueue, Pipeline<Frame> senderQueue) {
        this.socket = socket;
        this.running = running;
        this.udpConnection = udpConnection;
        this.receiverQueue = receiverQueue;
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
        byte[] ack = "\r".getBytes();
        byte[] data = new byte[1500];
        DatagramPacket packet = new DatagramPacket(data, data.length);

        try {
            while (isRunning()) {
                socket.receive(packet);
                senderQueue.add(new Frame(ack, packet.getAddress(), packet.getPort()));

                byte[] stream = new byte[packet.getLength()];
                System.arraycopy(packet.getData(), 0, stream, 0, packet.getLength());

                receiverQueue.add(new Frame(stream, packet.getAddress(), packet.getPort()));
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
