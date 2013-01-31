package main;

import main.conn.TCPConnection;
import main.conn.UDPConnection;
import main.data.Frame;
import main.utils.Pipeline;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.util.HashMap;
import java.util.Map;

public class Main {

    public static void main(String[] args) throws Exception {
        if (args.length < 4) {
            System.err.println("Usage: <tcp_ip> <tcp_port> <udp_port> <mode>\n * mode - \"run\" or \"test\"");
            return;
        }

        InetAddress tcpAddress = Inet4Address.getByName(args[0]);
        int tcpPort = Integer.parseInt(args[1]);

        InetAddress udpAddress = Inet6Address.getByName("::");
        int udpPort = Integer.parseInt(args[2]);

        String mode = args[3];

        Map<String, TCPConnection> tcpConnectionMap = new HashMap<String, TCPConnection>();
        UDPConnection udpConnection = new UDPConnection(udpAddress, udpPort);

        Pipeline<Frame> receiverQueue = udpConnection.getReceiverQueue();
        Pipeline<Frame> senderQueue = udpConnection.getSenderQueue();

        Frame frame;
        TCPConnection tcpConnection;

        if ("run".equals(mode)) {
            while ((frame = receiverQueue.remove()) != null) {
                String remote = frame.address.toString() + ":" + frame.port;

                tcpConnection = tcpConnectionMap.get(remote);
                if (tcpConnection == null || !tcpConnection.isRunning()) {
                    tcpConnection = new TCPConnection(tcpAddress, tcpPort, frame.address, frame.port, senderQueue);
                    tcpConnectionMap.put(remote, tcpConnection);
                }

                tcpConnection.getSenderQueue().add(frame);
            }
        } else if ("test".equals(mode)) {
            while ((frame = receiverQueue.remove()) != null) {
                senderQueue.add(frame);
            }
        }
    }

}
