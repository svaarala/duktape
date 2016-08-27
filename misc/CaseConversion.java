/*
 *  Helper to test Java case conversion (useful for comparison).
 */

import java.util.Locale;
import java.util.Properties;

public class CaseConversion {
    public static final String dump(String x) throws Exception {
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < x.length(); i++) {
            if (i != 0) { sb.append(" "); }
            sb.append(String.format("U+%04x", (int) x.charAt(i)));
        }
        return sb.toString();
    }

    public static final char getCharFromHex(String x) throws Exception {
        return Character.toChars(Integer.valueOf(x, 16))[0];
    }

    /* Parse string as is, except recognize backslash followed by 'u'
     * (4 hex digit escape), 'x' (2 hex digit escape) and another backslash
     * (backslash escape).  Note that when using this from a Unix shell,
     * backslashes need to be escaped for the shell too.
     */
    public static final String parseEscaped(String x) throws Exception {
        StringBuffer sb = new StringBuffer();
        for (int i = 0;;) {
            if (i == x.length()) {
                break;
            } else if (i > x.length()) {
                throw new Exception("invalid input");
            }
            char c = x.charAt(i);
            if (c != '\\') {
                sb.append(c);
                i++;
            } else {
                c = x.charAt(i + 1);
                if (c == 'x') {
                    sb.append(getCharFromHex(x.substring(i + 2, i + 4)));
                    i += 4;
                } else if (c == 'u') {
                    sb.append(getCharFromHex(x.substring(i + 2, i + 6)));
                    i += 6;
                } else if (c == '\\') {
                    sb.append('\\');
                    i += 2;
                } else {
                    throw new Exception("invalid input");
                }
            }
        }
        return sb.toString();
    }

    public static final void main(String args[]) throws Exception {
        Properties sysProps = System.getProperties();
        Locale locale = new Locale(args[0]);
        String inputString = parseEscaped(args[1]);
        String uc = inputString.toUpperCase(locale);
        String lc = inputString.toLowerCase(locale);

        System.out.println("java.version:    " + sysProps.getProperty("java.version"));
        System.out.println("java.vendor:     " + sysProps.getProperty("java.vendor"));
        System.out.println("java.vendor.url: " + sysProps.getProperty("java.vendor.url"));
        System.out.println("java.vm.version: " + sysProps.getProperty("java.vm.version"));
        System.out.println("java.vm.vendor:  " + sysProps.getProperty("java.vm.vendor"));
        System.out.println("java.vm.name:    " + sysProps.getProperty("java.vm.name"));
        System.out.println("Locale:          " + locale);
        System.out.println("Input:           " + dump(inputString));
        System.out.println("toUpperCase:     " + dump(uc));
        System.out.println("toLowerCase:     " + dump(lc));
    }
}
