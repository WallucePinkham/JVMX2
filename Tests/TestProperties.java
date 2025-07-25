
public class TestProperties {
    public static void main(String[] args) {
        // Retrieve and print common system properties
        String classPath = System.getProperty("java.class.path");
        String fileSeparator = System.getProperty("file.separator");
        String osName = System.getProperty("os.name");
        String osVersion = System.getProperty("os.version");
        String userName = System.getProperty("user.name");
        String userHome = System.getProperty("user.home");
        String javaVersion = System.getProperty("java.version");
        String javaVendor = System.getProperty("java.vendor");
        String javaHome = System.getProperty("java.home");
        String javaVendorUrl = System.getProperty("java.vendor.url");
        String lineSeparator = System.getProperty("line.separator");
        String pathSeparator = System.getProperty("path.separator");
       
        // Print the properties
        System.out.println("Java Class Path: " + classPath);
        System.out.println("File Separator: " + fileSeparator);
        System.out.println("Operating System Name: " + osName);
        System.out.println("Operating System Version: " + osVersion);
        System.out.println("User Name: " + userName);
        System.out.println("User Home Directory: " + userHome);
        System.out.println("Java Version: " + javaVersion);
        System.out.println("Java Vendor: " + javaVendor);
        System.out.println("Java Home Directory: " + javaHome);
        System.out.println("Java Vendor URL: " + javaVendorUrl);
        System.out.println("Line Separator: " + lineSeparator);
        System.out.println("Path Separator: " + pathSeparator);
    }
}