package main.utils.pipes;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import main.utils.Pipeline;


public class SinglePipeline<T> implements Pipeline<T> {

	private final List<T> objects;

	private final AtomicBoolean closed;

	public SinglePipeline() {
		this.objects = Collections.synchronizedList(new LinkedList<T>());
		this.closed = new AtomicBoolean(false);
	}

	@Override
	public boolean isClosed() {
		synchronized (closed) {
			return closed.get();
		}
	}

	@Override
	public void close() {
		synchronized (closed) {
			closed.set(true);
		}
	}

	@Override
	public void add(T obj) throws Exception {
		if (isClosed()) {
			throw new PipelineClosedException();
		}

		if (obj == null) {
			return;
		}

		synchronized (objects) {
			objects.add(obj);
			objects.notifyAll();
		}
	}
	
	@Override
	public T get() throws Exception {
		synchronized (objects) {
			while (objects.isEmpty()) {
				objects.wait();
			}
			return objects.get(0);
		}
	}

	@Override
	public T remove() throws Exception {
		synchronized (objects) {
			while (objects.isEmpty()) {
				objects.wait();
			}
			return objects.remove(0);
		}
	}

	@Override
	public boolean isEmpty() {
		synchronized (objects) {
			return objects.isEmpty();
		}
	}
}