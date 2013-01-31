package main.utils;


public interface Pipeline<T> {

    public void add(T obj) throws Exception;

    public T get() throws Exception;

    public T remove() throws Exception;

    public boolean isEmpty();

    public boolean isClosed();

    public void close();

}
