import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.nio.charset.UnsupportedCharsetException;
import java.util.Enumeration;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.io.Reader;
//import gnu.java.lang.CPStringBuilder;

public class TestLoadProperties {

    public static void main(String[] args) {
        // testLoadValidProperties();
        // testLoadEmptyInputStream();
        // testLoadMalformedInput();
        // testLoadWithComments();
        // testLoadWithWhitespace();
        // testLoadDuplicateKeys();
        // testLoadNullInputStream();
        testLogManagarCode();
        //System.out.println("anything");

        //testSomething();
    }

    private static void put(String key, String value) {
        System.out.println("key " + key + " value: " + value);
    }

    private static void testSomething() {
        try {

            String defaultConfig = "handlers = java.util.logging.ConsoleHandler   \n"
                    + ".level=INFO\n";
            InputStream inputStream = new ByteArrayInputStream(defaultConfig.getBytes("UTF-8"));
            Reader inReader = new InputStreamReader(inputStream, "ISO-8859-1");
            BufferedReader reader = new BufferedReader(inReader);
            String line;

            while ((line = reader.readLine()) != null) {
                char c = 0;
                int pos = 0;
                // Leading whitespaces must be deleted first.
                while (pos < line.length()
                        && Character.isWhitespace(c = line.charAt(pos)))
                    pos++;

                // If empty line or begins with a comment character, skip this line.
                if ((line.length() - pos) == 0
                        || line.charAt(pos) == '#' || line.charAt(pos) == '!')
                    continue;

                // The characters up to the next Whitespace, ':', or '='
                // describe the key. But look for escape sequences.
                // Try to short-circuit when there is no escape char.
                int start = pos;
                boolean needsEscape = line.indexOf('\\', pos) != -1;
                StringBuilder key = needsEscape ? new StringBuilder() : null;
                while (pos < line.length()
                        && !Character.isWhitespace(c = line.charAt(pos++))
                        && c != '=' && c != ':') {
                    if (needsEscape && c == '\\') {
                        if (pos == line.length()) {
                            // The line continues on the next line. If there
                            // is no next line, just treat it as a key with an
                            // empty value.
                            line = reader.readLine();
                            if (line == null)
                                line = "";
                            pos = 0;
                            while (pos < line.length()
                                    && Character.isWhitespace(c = line.charAt(pos)))
                                pos++;
                        } else {
                            c = line.charAt(pos++);
                            switch (c) {
                                case 'n':
                                    key.append('\n');
                                    break;
                                case 't':
                                    key.append('\t');
                                    break;
                                case 'r':
                                    key.append('\r');
                                    break;
                                case 'u':
                                    if (pos + 4 <= line.length()) {
                                        char uni = (char) Integer.parseInt(line.substring(pos, pos + 4), 16);
                                        key.append(uni);
                                        pos += 4;
                                    } // else throw exception?
                                    break;
                                default:
                                    key.append(c);
                                    break;
                            }
                        }
                    } else if (needsEscape)
                        key.append(c);
                }

                boolean isDelim = (c == ':' || c == '=');

                String keyString;
                if (needsEscape)
                    keyString = key.toString();
                else if (isDelim || Character.isWhitespace(c))
                    keyString = line.substring(start, pos - 1);
                else
                    keyString = line.substring(start, pos);

                while (pos < line.length()
                        && Character.isWhitespace(c = line.charAt(pos)))
                    pos++;

                if (!isDelim && (c == ':' || c == '=')) {
                    pos++;
                    while (pos < line.length()
                            && Character.isWhitespace(c = line.charAt(pos)))
                        pos++;
                }

                // Short-circuit if no escape chars found.
                if (!needsEscape) {
                    put(keyString, line.substring(pos));
                    continue;
                }

                // Escape char found so iterate through the rest of the line.
                StringBuilder element = new StringBuilder(line.length() - pos);
                while (pos < line.length()) {
                    c = line.charAt(pos++);
                    if (c == '\\') {
                        if (pos == line.length()) {
                            // The line continues on the next line.
                            line = reader.readLine();

                            // We might have seen a backslash at the end of
                            // the file. The JDK ignores the backslash in
                            // this case, so we follow for compatibility.
                            if (line == null)
                                break;

                            pos = 0;
                            while (pos < line.length()
                                    && Character.isWhitespace(c = line.charAt(pos)))
                                pos++;
                            element.ensureCapacity(line.length() - pos +
                                    element.length());
                        } else {
                            c = line.charAt(pos++);
                            switch (c) {
                                case 'n':
                                    element.append('\n');
                                    break;
                                case 't':
                                    element.append('\t');
                                    break;
                                case 'r':
                                    element.append('\r');
                                    break;
                                case 'u':
                                    if (pos + 4 <= line.length()) {
                                        char uni = (char) Integer.parseInt(line.substring(pos, pos + 4), 16);
                                        element.append(uni);
                                        pos += 4;
                                    } // else throw exception?
                                    break;
                                default:
                                    element.append(c);
                                    break;
                            }
                        }
                    } else
                        element.append(c);
                }
                put(keyString, element.toString());
            }
        } catch (IOException x) {

        }
    }

    private static void testLogManagarCode() {
        try {
            String defaultConfig = "handlers = java.util.logging.ConsoleHandler   \n"
                    + ".level=INFO\n";
            InputStream inputStream = new ByteArrayInputStream(defaultConfig.getBytes("UTF-8"));
            Properties properties = new Properties();
            properties.load(inputStream);

            Enumeration<?> keys = properties.propertyNames();

            while (keys.hasMoreElements()) {
                String key = ((String) keys.nextElement()).trim();
                getLevel(properties, key);
            }
        } catch (IOException x) {
            System.out.println("IOException occurred: " + x.getMessage());
        }
    }

    private static void getLevel(Properties properties, String key) {
        String value = properties.getProperty(key);

        // System.out.println("key: " + key);
        // System.out.println("value: " + value);

        // if (value == null)
        // continue;

        // value = value.trim();

        // if ("handlers".equals(key)) {
        // StringTokenizer tokenizer = new StringTokenizer(value, " \t\n\r\f,");
        // while (tokenizer.hasMoreTokens()) {
        // String handlerName = tokenizer.nextToken();
        // System.out.println("handler: " + handlerName);
        // }
        // }

        if (key.endsWith(".level")) {
            String loggerName = key.substring(0, key.length() - 6);
            System.out.println("logger: " + loggerName);
            Level level = null;
            try {
                System.out.println("value: " + value);
                level = Level.parse(value);
            } catch (IllegalArgumentException e) {
                System.out.println("bad level \'" + value + "\'");
            }
        }
    }

    private static void testLoadDefaultConfig() {
        String defaultConfig = "handlers = java.util.logging.ConsoleHandler   \n"
                + ".level=INFO \n";
        try (InputStream inputStream = new ByteArrayInputStream(defaultConfig.getBytes("UTF-8"))) {
            Properties properties = new Properties();
            properties.load(inputStream);

            // Check that the properties are loaded correctly
            assertEqual("java.util.logging.ConsoleHandler", properties.getProperty("handlers").trim(),
                    "testLoadDefaultConfig");
            assertEqual("INFO", properties.getProperty(".level").trim(), "testLoadDefaultConfig");
        } catch (IOException e) {
            System.out.println("IOException in testLoadDefaultConfig: " + e.getMessage());
        }
    }

    private static void testLoadValidProperties() {
        String propertiesContent = "key1=value1\nkey2=value2";
        try (InputStream inputStream = new ByteArrayInputStream(propertiesContent.getBytes("UTF-8"))) {
            Properties properties = new Properties();
            properties.load(inputStream);

            assertEqual("value1", properties.getProperty("key1"), "testLoadValidProperties");
            assertEqual("value2", properties.getProperty("key2"), "testLoadValidProperties");
        } catch (IOException e) {
            System.out.println("IOException in testLoadValidProperties: " + e.getMessage());
        }
    }

    private static void testLoadEmptyInputStream() {
        try (InputStream inputStream = new ByteArrayInputStream(new byte[0])) {
            Properties properties = new Properties();
            properties.load(inputStream);

            assertEqual(true, properties.isEmpty(), "testLoadEmptyInputStream");
        } catch (IOException e) {
            System.out.println("IOException in testLoadEmptyInputStream: " + e.getMessage());
        }
    }

    private static void testLoadMalformedInput() {
        String propertiesContent = "key1=value1\nkey2="; // key2 has no value
        try (InputStream inputStream = new ByteArrayInputStream(propertiesContent.getBytes("UTF-8"))) {
            Properties properties = new Properties();
            properties.load(inputStream);

            assertEqual("value1", properties.getProperty("key1"), "testLoadMalformedInput");
            assertEqual("", properties.getProperty("key2"), "testLoadMalformedInput"); // key2 should be null
        } catch (IOException e) {
            System.out.println("IOException in testLoadMalformedInput: " + e.getMessage());
        }
    }

    private static void testLoadWithComments() {
        String propertiesContent = "# This is a comment\nkey1=value1\n# Another comment\nkey2=value2";
        try (InputStream inputStream = new ByteArrayInputStream(propertiesContent.getBytes("UTF-8"))) {
            Properties properties = new Properties();
            properties.load(inputStream);

            assertEqual("value1", properties.getProperty("key1"), "testLoadWithComments");
            assertEqual("value2", properties.getProperty("key2"), "testLoadWithComments");
        } catch (IOException e) {
            System.out.println("IOException in testLoadWithComments: " + e.getMessage());
        }
    }

    private static void testLoadWithWhitespace() {
        String propertiesContent = " key1 = value1 \n key2 = value2 ";
        try (InputStream inputStream = new ByteArrayInputStream(propertiesContent.getBytes("UTF-8"))) {
            Properties properties = new Properties();
            properties.load(inputStream);

            assertEqual("value1", properties.getProperty("key1").trim(), "testLoadWithWhitespace");
            assertEqual("value2", properties.getProperty("key2").trim(), "testLoadWithWhitespace");
        } catch (IOException e) {
            System.out.println("IOException in testLoadWithWhitespace: " + e.getMessage());
        }
    }

    private static void testLoadDuplicateKeys() {
        String propertiesContent = "key1=value1\nkey1=value2"; // Duplicate key
        try (InputStream inputStream = new ByteArrayInputStream(propertiesContent.getBytes("UTF-8"))) {
            Properties properties = new Properties();
            properties.load(inputStream);

            assertEqual("value2", properties.getProperty("key1"), "testLoadDuplicateKeys"); // Last value should
                                                                                            // overwrite
        } catch (IOException e) {
            System.out.println("IOException in testLoadDuplicateKeys: " + e.getMessage());
        }
    }

    private static void testLoadNullInputStream() {
        Properties properties = new Properties();
        try {
            properties.load((InputStream) null);
            System.out.println("testLoadNullInputStream failed: Expected NullPointerException");
        } catch (NullPointerException e) {
            System.out.println("testLoadNullInputStream passed: Caught expected NullPointerException");
        } catch (IOException e) {
            System.out.println("IOException in testLoadNullInputStream: " + e.getMessage());
        }
    }

    private static void assertEqual(Object expected, Object actual, String testName) {
        if (expected == null && actual == null) {
            System.out.println(testName + " passed.");
        } else if (expected != null && expected.equals(actual)) {
            System.out.println(testName + " passed.");
        } else {
            System.out.println(testName + " failed: expected " + expected + ", but got " + actual);
        }
    }
}
