public class AllTestsRunner {
    public static void main(String[] args) {
        System.out.println("Running TestProperties...");
        TestProperties.main(new String[0]);

        System.out.println("Running TestFloat...");
        TestFloat.main(new String[0]);

        System.out.println("Running TestHashTables...");
        TestHashTables.main(new String[0]);

        System.out.println("Running TestLanguageFeatures...");
        TestLanguageFeatures.main(new String[0]);

        System.out.println("Running TestLoadProperties...");
        TestLoadProperties.main(new String[0]);

        System.out.println("Running TestThreads...");
        TestThreads.main(new String[0]);

        System.out.println("Running TestUnhandledException...");
        try {
            TestUnhandledException.main(new String[0]);
        } catch (Throwable t) {
            System.out.println("Caught unhandled exception: " + t);
        }

        System.out.println("All tests finished.");
    }
}