package main;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.log4j.Logger;

import main.utils.Pipeline;
import main.utils.pipes.SinglePipeline;

public class UDPConnection {

	private static final Logger logger = Logger.getLogger(UDPConnection.class);

	private final Thread receiver = new Thread("udp-receiver") {

		@Override
		public void run() {
			byte[] data = new byte[1024];
			DatagramPacket packet = new DatagramPacket(data, data.length);

			logger.info("Starting receiver thread");
			try {
				while (isRunning()) {
					socket.receive(packet);

					byte[] tmp = new byte[packet.getLength()];
					System.arraycopy(packet.getData(), 0, tmp, 0, packet.getLength());

					logger.info("Sensor: " + packet.getAddress().toString() + ":" + packet.getPort());
					logger.debug("Data: " + new String(tmp));

					receiverQueue.add(new Frame(tmp, packet.getAddress(), packet.getPort()));
				}
			} catch (Exception e) {
				logger.error("Error during receiving frame", e);
				setRunning(false);
				sender.interrupt();
			}
			logger.info("Stopping receiver thread");
			close();
		}
	};

	private final Thread sender = new Thread("udp-sender") {

		@Override
		public void run() {
			DatagramPacket packet = null;
			Frame frame = null;

			logger.info("Starting sender thread");
			try {
				while (isRunning()) {
					frame = senderQueue.remove();
					packet = new DatagramPacket(frame.data, frame.data.length, frame.address, frame.port);
					socket.send(packet);

					logger.info("Sensor: " + frame.address + ":" + frame.port);
					logger.debug("Data: " + new String(frame.data));
				}
			} catch (Exception e) {
				logger.error("Error during sending frame", e);
				setRunning(false);
				receiver.interrupt();
			}
			logger.info("Stopping sender thread");
			close();
		}
	};

	private final AtomicBoolean running = new AtomicBoolean(false);

	private final Pipeline<Frame> senderQueue = new SinglePipeline<Frame>();

	private final Pipeline<Frame> receiverQueue = new SinglePipeline<Frame>();

	private final Object closing = new Object();

	private final DatagramSocket socket;

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

	public Pipeline<Frame> getReceiverQueue() {
		return receiverQueue;
	}

	public Pipeline<Frame> getSenderQueue() {
		return senderQueue;
	}

	public UDPConnection(int port) throws Exception {
		InetAddress address = Inet6Address.getByName("::");
		SocketAddress addr = new InetSocketAddress(address, port);
		socket = new DatagramSocket(addr);
		logger.info("Bound to " + addr.toString());

		setRunning(true);
		receiver.start();
		sender.start();
	}

	private void close() {
		synchronized (closing) {
			if (socket.isBound() && !socket.isClosed()) {
				socket.close();
				logger.info("Closed");
			}
		}
	}

	public void stop() {
		setRunning(false);
		sender.interrupt();
		receiver.interrupt();
		close();
	}
}
