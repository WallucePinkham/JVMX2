/* VMString.java -- VM Specific String methods
   Copyright (C) 2024  JVMX project.

This file is part of JVMX. It was adapted from GNU ClassPath.
See the README.md and LICENSE files for JVMX and the LICENSE file for GNU ClassPath.

This is a custom implementation of the VMString class to handle the "intern"-ing of strings.

 */
 
package java.lang;


final class VMString
{
  private VMString() {} // Prohibits instantiation.

  native static String intern(String str);

} // class VMString
