public class TestThreads {
    public static void main(String[] args) {

        System.out.println("Starting Thread Tests");

        threadCreationTest();
        threadSynchronizationTest();
        threadCommunicationTest();
    }

    public static void threadCreationTest() {
        Thread thread1 = new Thread(new Runnable() {
            @Override
            public void run() {
                System.out.println("Thread 1 is running");
            }
        });
        thread1.start();

        Thread thread2 = new Thread(new Runnable() {
            @Override
            public void run() {
                System.out.println("Thread 2 is running");
            }
        });
        thread2.start();

        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
            // TODO: handle exception
        }

        // Add more thread creation and start tests as needed
    }

    public static void threadSynchronizationTest() {
        final Object lock = new Object();
        final int[] sharedResource = { 0 };

        Thread thread1 = new Thread(new Runnable() {
            @Override
            public void run() {
                synchronized (lock) {
                    sharedResource[0]++;
                    System.out.println("Thread 1: Shared resource incremented to " + sharedResource[0]);
                }
            }
        });

        Thread thread2 = new Thread(new Runnable() {
            @Override
            public void run() {
                synchronized (lock) {
                    sharedResource[0]--;
                    System.out.println("Thread 2: Shared resource decremented to " + sharedResource[0]);
                }
            }
        });

        thread1.start();
        thread2.start();

        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
            // TODO: handle exception
        }

        // Add more synchronization tests as needed
    }

    public static void threadCommunicationTest() {
        final Object lock = new Object();
        final boolean[] conditionMet = { false };

        Thread waitingThread = new Thread(new Runnable() {
            @Override
            public void run() {
                synchronized (lock) {
                    while (!conditionMet[0]) {
                        try {
                            lock.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                    System.out.println("Waiting thread resumed");
                }
            }
        });

        Thread notifyingThread = new Thread(new Runnable() {
            @Override
            public void run() {
                synchronized (lock) {
                    conditionMet[0] = true;
                    lock.notify();
                    System.out.println("Notifying thread sent notification");
                }
            }
        });

        waitingThread.start();
        notifyingThread.start();
        try {
            waitingThread.join();
            notifyingThread.join();
        } catch (InterruptedException e) {
            // TODO: handle exception
        }

        // Add more thread communication tests as needed
    }
}