package main.utils;

public class Atomic<T> {

	private T object;

	public Atomic() {
		set(null);
	}

	public Atomic(T object) {
		set(object);
	}

	public void set(T object) {
		synchronized (this) {
			this.object = object;
		}
	}

	public T get() {
		synchronized (this) {
			return object;
		}
	}

}
