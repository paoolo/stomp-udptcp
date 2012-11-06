package main;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.log4j.Logger;

import main.utils.Pipeline;
import main.utils.pipes.SinglePipeline;

public class TCPConnection {

	private static final Logger logger = Logger.getLogger(TCPConnection.class);

	private final Thread receiver = new Thread("tcp-receiver") {

		@Override
		public void run() {
			byte[] data = new byte[1024];

			logger.info("Starting receiver thread");
			try {
				InputStream in = socket.getInputStream();
				while (isRunning()) {
					int length = in.read(data);

					byte[] tmp = new byte[length];
					System.arraycopy(data, 0, tmp, 0, length);

					logger.info("Broker: " + tcpHostname + ":" + udpPort);
					logger.debug("Data: " + tmp);

					receiverQueue.add(new Frame(data, udpAddress, udpPort));
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

	private final Thread sender = new Thread("tcp-sender") {

		@Override
		public void run() {
			Frame frame = null;

			logger.info("Starting sender thread");
			try {
				OutputStream out = socket.getOutputStream();
				while (isRunning()) {
					frame = senderQueue.remove();
					out.write(frame.data);

					logger.info("Broker: " + tcpHostname + ":" + udpPort);
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

	private final Pipeline<Frame> receiverQueue;

	private final Object closing = new Object();

	private final String tcpHostname;

	private final int tcpPort;

	private final Socket socket;

	private final InetAddress udpAddress;

	private final int udpPort;

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

	public TCPConnection(String hostname, int port, InetAddress udpAddress, int udpPort, Pipeline<Frame> receiverQueue) throws Exception {
		this.tcpHostname = hostname;
		this.tcpPort = port;
		this.udpAddress = udpAddress;
		this.udpPort = udpPort;
		this.receiverQueue = receiverQueue;

		InetAddress address = Inet4Address.getByName(tcpHostname);
		SocketAddress endpoint = new InetSocketAddress(address, tcpPort);
		socket = new Socket();
		socket.connect(endpoint);
		logger.info("Connected to " + endpoint.toString());

		setRunning(true);
		receiver.start();
		sender.start();
	}

	private void close() {
		synchronized (closing) {
			if (socket.isConnected() && !socket.isConnected()) {
				try {
					socket.close();
					logger.info("Closed");
				} catch (IOException e) {
					logger.warn("Error during closing socket", e);
				}
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
