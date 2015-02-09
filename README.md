maskprocessor
==============

High-Performance word generator with a per-position configureable charset

Mask attack
--------------

Try all combinations from a given keyspace just like in Brute-Force attack, but more specific.

Advantage over Brute-Force
--------------

The reason for doing this and not to stick to the traditional Brute-Force is that we want to reduce the password candidate keyspace to a more efficient one.

Here is a single example. We want to crack the password: Julia1984

In traditional Brute-Force attack we require a charset that contains all upper-case letters, all lower-case letters and all digits (aka “mixalpha-numeric”). The Password length is 9, so we have to iterate through 62^9 (13.537.086.546.263.552) combinations. Lets say we crack with a rate of 100M/s, this requires more than 4 years to complete.

In Mask attack we know about humans and how they design passwords. The above password matches a simple but common pattern. A name and year appended to it. We can also configure the attack to try the upper-case letters only on the first position. It is very uncommon to see an upper-case letter only in the second or the third position. To make it short, with Mask attack we can reduce the keyspace to 52*26*26*26*26*10*10*10*10 (237.627.520.000) combinations. With the same cracking rate of 100M/s, this requires just 40 minutes to complete.

Disadvantage compared to Brute-Force
--------------

There is none. One can argue that the above example is very specific but this does not matter. Even in mask attack we can configure our mask to use exactly the same keyspace as the Brute-Force attack does. The thing is just that this cannot work vice versa.

Masks
--------------

For each position of the generated password candidates we need to configure a placeholder. If a password we want to crack has the length 8, our mask must consist of 8 placeholders.

A mask is a simple string that configures the keyspace of the password candidate engine using placeholders.
A placeholder can be either a custom charset variable, a built-in charset variable or a static letter.
A variable is indicated by the ? letter followed by one of the built-in charset (l, u, d, s, a) or one of the custom charset variable names (1, 2, 3, 4).
A static letter is not indicated by a letter. An exception is if we want the static letter ? itself, which must be written as ??.

Built-in charsets
--------------

?l = abcdefghijklmnopqrstuvwxyz
?u = ABCDEFGHIJKLMNOPQRSTUVWXYZ
?d = 0123456789
?s = «space»!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~
?a = ?l?u?d?s
?b = 0x00 - 0xff

Custom charsets
--------------

There are four commandline-parameters to configure four custom charsets.

--custom-charset1=CS
--custom-charset2=CS
--custom-charset3=CS
--custom-charset4=CS

These commandline-parameters have four analogue shortcuts called -1, -2, -3 and -4. You can specify the chars directly on the command line.

Password length increment
--------------

A Mask attack is always specific to a password length. For example, if we use the mask ”?l?l?l?l?l?l?l?l” we can only crack a password of the length 8. But if the password we try to crack has the length 7 we will not find it. Thats why we have to repeat the attack several times, each time with one placeholder added to the mask. This is transparently automated by using the ”--increment” flag.

?l
?l?l
?l?l?l
?l?l?l?l
?l?l?l?l?l
?l?l?l?l?l?l
?l?l?l?l?l?l?l
?l?l?l?l?l?l?l?l

Performance
--------------

Currently, it is the world's fastest word generator. Here are some stats:

AMD Athlon™ 64 X2 Dual Core Processor 3800+: 75.80 M/s (per core)
AMD FX™-6100 Six-Core Processor: 138.20 M/s (per core)
Intel(R) Xeon(R) CPU X5650 @ 2.67GHz: 97.42 M/s (per core)
Intel(R) i7-920: 71.50 M/s (per core)

To avoid irregularities while testing, all output went into /dev/null.

Example
--------------

The following commands creates the following password candidates:

command: ?l?l?l?l?l?l?l?l
keyspace: aaaaaaaa - zzzzzzzz
command: -1 ?l?d ?1?1?1?1?1
keyspace: aaaaa - 99999
command: password?d
keyspace: password0 - password9
command: -1 ?l?u ?1?l?l?l?l?l19?d?d
keyspace: aaaaaa1900 - Zzzzzz1999
command: -1 ?dabcdef -2 ?l?u ?1?1?2?2?2?2?2
keyspace: 00aaaaa - ffZZZZZ
command: -1 efghijklmnop ?1?1?1
keyspace: eee - ppp

Compile
--------------

Simply run make

Binary distribution
--------------

Binaries for Linux, Windows and OSX: https://github.com/jsteube/maskprocessor/releases
