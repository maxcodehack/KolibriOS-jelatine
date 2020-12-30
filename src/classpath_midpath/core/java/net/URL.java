package java.net;

import java.util.Hashtable;

/*public class URL extends jelatine.cldc.io.URL{

};
*/


/**
 * This class stores URL format as described in RFC 2396. This takes the general
 * form:<br>
 * {scheme}:[{target}][{params}]<br>
 * where {scheme} is the name of a protocol such as http.<br>
 * The {target} is normally some kind of network address.<br>
 * Any {params} are formed as a series of equates of the form ";x=y". Example:
 * ";type=a".
 */

public class URL {

    String name; /**< The URL name */
    String scheme; /**< The URL scheme */
    String target; /**< The URL target */
    String params; /**< The URL parameters */

    /** Default constructor for an URL
     * @param name A string holding the URL */

    public URL(String name) throws IllegalArgumentException {
        this.name = name;

        int shemeEndIndex = name.indexOf(":");

        if (shemeEndIndex == -1) {
            throw new IllegalArgumentException("Invalid URL");
        }

        scheme = name.substring(0, shemeEndIndex);
        int paramsStartIndex = name.indexOf(";");

        if (paramsStartIndex == -1) {
            target = name.substring(shemeEndIndex + 1);
        } else {
            target = name.substring(shemeEndIndex + 1, paramsStartIndex);
            params = name.substring(paramsStartIndex);
        }
    }

    /** Parses the parameters held by \a params and returns them in an hash
     * table
     * @param params a string holding the URL parameters
     * @returns The parameters parsed in an hash-table */

    public Hashtable getParamsMap(String params) {
        Hashtable map = new Hashtable();

        int paramStartIndex = -1;

        while ((paramStartIndex = params.indexOf(';', paramStartIndex + 1))
               != -1)
        {
            int paramEndIndex = params.indexOf(';', paramStartIndex + 1);
            String keyValueString;

            if (paramEndIndex == -1) {
                keyValueString = params.substring(paramStartIndex + 1);
            } else {
                keyValueString = params.substring(paramStartIndex + 1,
                                                  paramEndIndex);
            }

            int equalIndex = keyValueString.indexOf('=');

            if (equalIndex == -1) {
                break;
            }

            String key = keyValueString.substring(0, equalIndex);
            String value = keyValueString.substring(equalIndex + 1);
            map.put(key, value);
        }

        return map;
    }

    /** Return the URL name
     * @returns The URL name as a string */

    public String getName() {
        return name;
    }

    /** Return the URL scheme
     * @returns The URL scheme as a string */

    public String getScheme() {
        return scheme;
    }

    /** Return the URL target
     * @returns The URL target as a string */

    public String getTarget() {
        return target;
    }

    /** Return the URL parameters
     * @returns The URL parameters as a string */

    public String getParams() {
        return params;
    }

}
