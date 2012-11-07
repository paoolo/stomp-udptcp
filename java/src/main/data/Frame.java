package main.data;

import java.net.InetAddress;

public class Frame {

	public final byte[] data;
	
	public final InetAddress address;
	
	public final int port;
	
	public Frame(byte[] frame, InetAddress address, int port) {
		this.data = frame;
		this.address = address;
		this.port = port;
	}
}
