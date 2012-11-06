package main;

import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;

import main.utils.Pipeline;

public class Main {

	private static final Logger logger = Logger.getLogger(Main.class);
	
	public static void main(String[] args) throws Exception {
		if (args.length < 3) {
			System.err.println("Usage: <tcp_ip> <tcp_port> <udp_port>");
			return;
		}
		
		BasicConfigurator.configure();

		String tcpHostname = args[0];
		int tcpPort = Integer.parseInt(args[1]);
		int udpPort = Integer.parseInt(args[2]);

		Map<String, TCPConnection> tcpConnectionMap = new HashMap<String, TCPConnection>();
		UDPConnection udpConnection = new UDPConnection(udpPort);

		Pipeline<Frame> receiverQueue = udpConnection.getReceiverQueue();
		Pipeline<Frame> senderQueue = udpConnection.getSenderQueue();

		Frame frame = null;
		TCPConnection tcpConnection = null;

		while ((frame = receiverQueue.remove()) != null) {
			String remote = frame.address.toString() + ":" + frame.port;

			logger.info("Sensor: " + remote + " Data: " + new String(frame.data));

			if (!tcpConnectionMap.containsKey(remote)) {
				tcpConnection = new TCPConnection(tcpHostname, tcpPort, frame.address, frame.port, senderQueue);
				tcpConnectionMap.put(remote, tcpConnection);
			} else {
				tcpConnection = tcpConnectionMap.get(remote);
			}
			tcpConnection.getSenderQueue().add(frame);
		}
	}

}
