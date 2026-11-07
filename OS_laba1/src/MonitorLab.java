// Класс, который представляет само "событие" или
// данные, передаваемые от поставщика к потребителю
class EventData {
    private final int id;
    private final String message;

    public EventData(int id, String message) {
        this.id = id;
        this.message = message;
    }

    @Override
    public String toString() {
        return "Событие #" + id + "[" + message + "]";
    }
}

class Monitor {
    private volatile boolean ready = false;
    private EventData data = null;

    public synchronized void provide(EventData newData) {
        while (ready) {
            try {
                wait();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            }
        }

        data = newData;
        ready = true;
        System.out.println("Поставщик: Отправлено -> " + data);

        notifyAll();
    }

    public synchronized EventData consume() {
        while(!ready) {
            try {
                wait();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return null;
            }
        }

        EventData consumeData = data;
        ready = false;
        System.out.println("Потребитель: Обработано <- " + consumeData);

        notifyAll();
        return consumeData;
    }
}

class Producer implements Runnable {
    private final Monitor monitor;
    private int eventId = 1;

    public Producer(Monitor monitor) {
        this.monitor = monitor;
    }

    @Override
    public void run() {
        try {
            while (!Thread.currentThread().isInterrupted()) {
                Thread.sleep(1000);

                EventData newEvent = new EventData(eventId++, "Данные полезной нагрузки");
                monitor.provide(newEvent);
            }
        } catch (InterruptedException e) {
            System.out.println("Поставщик завершил работу.");
        }
    }
}

class Consumer implements Runnable {
    private final Monitor monitor;

    public Consumer(Monitor monitor) {
        this.monitor = monitor;
    }

    @Override
    public void run() {
        try {
            while (!Thread.currentThread().isInterrupted()) {
                monitor.consume();
                Thread.sleep(100);
            }
        } catch (InterruptedException e) {
            System.out.println("Потребитель завершил работу");
        }
    }
}

public class MonitorLab {
    public static void main(String[] args) {
        Monitor sharedMonitor = new Monitor();
        Thread producerThread = new Thread(new Producer(sharedMonitor), "Producer-Thread");
        Thread consumerThread = new Thread(new Consumer(sharedMonitor), "Consumer-Thread");

        producerThread.start();
        consumerThread.start();

        try {
            Thread.sleep(10000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        producerThread.interrupt();
        consumerThread.interrupt();
    }
}
