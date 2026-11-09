import java.io.File;

public class TestJNIEntrypoints {
    static {
        System.load(new File("..\\tests\\TestJNIEntrypoints.dll").getAbsolutePath());
    }

    private int intField = 17;
    private long longField = 29;
    private Object objectField = "field";

    private Object objectMethod() {
        return this;
    }

    private void voidMethod() {
    }

    public static void main(String[] args) {
        System.out.println("Running TestJNIEntrypoints...");

        long result = runAll();
        int total = (int) (result >>> 32);
        int failed = (int) result;
        System.out.println("JNI entry point tests: " + total + " total, " + failed + " failed.");
    }

    private static native long runAll();
}
