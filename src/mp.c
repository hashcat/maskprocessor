#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdint.h>

/**
 * Name........: maskprocesspr (mp)
 * Description.: High-Performance word generator with a per-position configureable charset
 * Version.....: 0.73
 * Autor.......: Jens Steube <jens.steube@gmail.com>
 * License.....: MIT
 */

#define CHARSIZ          0x100
#define PW_MAX           0x100
#define SPARE            100
#define OUTBUFSZ         BUFSIZ
#define VERSION_BIN      73
#define SEQ_MAX          0
#define OCCUR_MAX        0
#define OCCUR_CACHE_SIZE 3
#define INCREMENT        0
#define INCREMENT_MIN    1
#define INCREMENT_MAX    PW_MAX

typedef struct __cs
{
  char  cs_buf[CHARSIZ];
  char  cs_uniq[CHARSIZ];
  int   cs_len;
  int   cs_pos;
  int   buf_pos;

} cs_t;

typedef struct
{
  char buf[OUTBUFSZ + SPARE];

  int pos;

} out_t;

static const char *USAGE_MINI[] =
{
  "Usage: %s [options] mask",
  "",
  "Try --help for more help.",
  NULL
};

static const char *USAGE_BIG[] =
{
  "High-Performance word generator with a per-position configureable charset",
  "",
  "Usage: %s [options]... mask",
  "",
  "* Startup:",
  "",
  "  -V,  --version             Print version",
  "  -h,  --help                Print help",
  "",
  "* Increment:",
  "",
  "  -i,  --increment=NUM:NUM   Enable increment mode. 1st NUM=start, 2nd NUM=stop",
  "                             Example: -i 4:8 searches lengths 4-8 (inclusive)",
  "",
  "* Misc:",
  "",
  "       --combinations        Calculate number of combinations",
  "       --hex-charset         Assume charset is given in hex",
  "  -q,  --seq-max=NUM         Maximum number of multiple sequential characters",
  "  -r,  --occurrence-max=NUM  Maximum number of occurrence of a character",
  "",
  "* Resources:",
  "",
  "  -s,  --start-at=WORD       Start at specific position",
  "  -l,  --stop-at=WORD        Stop at specific position",
  "",
  "* Files:",
  "",
  "  -o,  --output-file=FILE    Output-file",
  "",
  "* Custom charsets:",
  "",
  "  -1,  --custom-charset1=CS  User-defineable charsets",
  "  -2,  --custom-charset2=CS  Example:",
  "  -3,  --custom-charset3=CS  --custom-charset1=?dabcdef",
  "  -4,  --custom-charset4=CS  sets charset ?1 to 0123456789abcdef",
  "",
  "* Built-in charsets:",
  "",
  "  ?l = abcdefghijklmnopqrstuvwxyz",
  "  ?u = ABCDEFGHIJKLMNOPQRSTUVWXYZ",
  "  ?d = 0123456789",
  "  ?s =  !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
  "  ?a = ?l?u?d?s",
  "  ?b = 0x00 - 0xff",
  "",
  NULL
};

static void usage_mini_print (const char *progname)
{
  int i;

  for (i = 0; USAGE_MINI[i] != NULL; i++)
  {
    printf (USAGE_MINI[i], progname);

    #ifdef OSX
    putchar ('\n');
    #endif

    #ifdef LINUX
    putchar ('\n');
    #endif

    #ifdef WINDOWS
    putchar ('\r');
    putchar ('\n');
    #endif
  }
}

static void usage_big_print (const char *progname)
{
  int i;

  for (i = 0; USAGE_BIG[i] != NULL; i++)
  {
    printf (USAGE_BIG[i], progname);

    #ifdef OSX
    putchar ('\n');
    #endif

    #ifdef LINUX
    putchar ('\n');
    #endif

    #ifdef WINDOWS
    putchar ('\r');
    putchar ('\n');
    #endif
  }
}

static char hex_convert (char c)
{
  return (c & 15) + (c >> 6) * 9;
}

static void add_cs_buf (const char *input_buf, cs_t *css, const int css_cnt)
{
  size_t input_len = strlen (input_buf);

  int pos = 0;

  size_t i;

  for (i = 0; i < input_len; i++)
  {
    int u = input_buf[i];

    if (css[css_cnt].cs_uniq[u] == 1) continue;

    css[css_cnt].cs_uniq[u] = 1;
    css[css_cnt].cs_buf[pos] = input_buf[i];

    pos++;
  }

  css[css_cnt].cs_len = pos;
  css[css_cnt].cs_pos = 0;
  css[css_cnt].buf_pos = css_cnt;
}

uint64_t mp_get_sum (int css_cnt, cs_t *css)
{
  uint64_t sum = 1;

  int css_pos;

  for (css_pos = 0; css_pos < css_cnt; css_pos++)
  {
    sum *= css[css_pos].cs_len;
  }

  return (sum);
}

static size_t mp_expand (const char *in_buf, const size_t in_len, char *out_buf, char *mp_sys[6], int hex_charset)
{
  size_t in_pos;

  for (in_pos = 0; in_pos < in_len; in_pos++)
  {
    char p0 = in_buf[in_pos];

    if (p0 == '?')
    {
      in_pos++;

      char p1 = in_buf[in_pos];

      switch (p1)
      {
        case 'l': strcat (out_buf, mp_sys[0]);
                  break;
        case 'u': strcat (out_buf, mp_sys[1]);
                  break;
        case 'd': strcat (out_buf, mp_sys[2]);
                  break;
        case 's': strcat (out_buf, mp_sys[3]);
                  break;
        case 'a': strcat (out_buf, mp_sys[4]);
                  break;
        case 'b': memcpy (out_buf, mp_sys[5], 256);
                  break;
        case '?': strcat (out_buf, "?");
                  break;
        default:  fprintf (stderr, "ERROR: Syntax Error: %s\n", in_buf);
                  exit (-1);
      }
    }
    else
    {
      if (hex_charset)
      {
        in_pos++;

        if (in_pos == in_len) break;

        char p1 = in_buf[in_pos];

        char s[2] = { 0, 0 };

        s[0]  = hex_convert (p1) << 0;
        s[0] |= hex_convert (p0) << 4;

        strcat (out_buf, s);
      }
      else
      {
        char s[2] = { p0, 0 };

        strcat (out_buf, s);
      }
    }
  }

  return strlen (out_buf);
}

static int next (char *word_buf, cs_t *css_buf, const int pos, int *occurs)
{
  int i;

  for (i = pos - 1; i >= 0; i--)
  {
    const int c_old = word_buf[i];

    occurs[c_old]--;

    cs_t *cs = css_buf + i;

    if (cs->cs_pos < cs->cs_len)
    {
      const int c_new = cs->cs_buf[cs->cs_pos];

      occurs[c_new]++;

      word_buf[i] = (char) c_new;

      cs->cs_pos++;

      return i;
    }

    const int c_new = cs->cs_buf[0];

    occurs[c_new]++;

    word_buf[i] = (char) c_new;

    cs->cs_pos = 1;
  }

  return -1;
}

static int find_pos (cs_t *cs, char c)
{
  int i;

  for (i = 0; i < cs->cs_len; i++)
  {
    if (cs->cs_buf[i] == c) return i;
  }

  return -1;
}

int main (int argc, char *argv[])
{
  /* usage */

  int   version           = 0;
  int   usage             = 0;
  int   combinations      = 0;
  int   hex_charset       = 0;
  int   seq_max           = SEQ_MAX;
  int   occur_max         = OCCUR_MAX;
  int   increment         = INCREMENT;
  char *increment_data    = NULL;
  int   increment_min     = INCREMENT_MIN;
  int   increment_max     = INCREMENT_MAX;
  char *start_at          = NULL;
  char *stop_at           = NULL;
  char *output_file       = NULL;
  char *custom_charset_1  = NULL;
  char *custom_charset_2  = NULL;
  char *custom_charset_3  = NULL;
  char *custom_charset_4  = NULL;

  #define IDX_VERSION           'V'
  #define IDX_USAGE             'h'
  #define IDX_HEX_CHARSET       0
  #define IDX_COMBINATIONS      1
  #define IDX_INCREMENT         'i'
  #define IDX_START_AT          's'
  #define IDX_STOP_AT           'l'
  #define IDX_OUTPUT_FILE       'o'
  #define IDX_SEQ_MAX           'q'
  #define IDX_OCCUR_MAX         'r'
  #define IDX_CUSTOM_CHARSET_1  '1'
  #define IDX_CUSTOM_CHARSET_2  '2'
  #define IDX_CUSTOM_CHARSET_3  '3'
  #define IDX_CUSTOM_CHARSET_4  '4'

  struct option long_options[] =
  {
    {"version",         no_argument,       0, IDX_VERSION},
    {"help",            no_argument,       0, IDX_USAGE},
    {"hex-charset",     no_argument,       0, IDX_HEX_CHARSET},
    {"combinations",    no_argument,       0, IDX_COMBINATIONS},
    {"seq-max",         required_argument, 0, IDX_SEQ_MAX},
    {"occur-max",       required_argument, 0, IDX_OCCUR_MAX},
    {"increment",       required_argument, 0, IDX_INCREMENT},
    {"start-at",        required_argument, 0, IDX_START_AT},
    {"stop-at",         required_argument, 0, IDX_STOP_AT},
    {"output-file",     required_argument, 0, IDX_OUTPUT_FILE},
    {"custom-charset1", required_argument, 0, IDX_CUSTOM_CHARSET_1},
    {"custom-charset2", required_argument, 0, IDX_CUSTOM_CHARSET_2},
    {"custom-charset3", required_argument, 0, IDX_CUSTOM_CHARSET_3},
    {"custom-charset4", required_argument, 0, IDX_CUSTOM_CHARSET_4},
    {0, 0, 0, 0}
  };

  int option_index = 0;

  int c;

  while ((c = getopt_long (argc, argv, "Vhi:q:s:l:o:1:2:3:4:r:", long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case IDX_VERSION:           version           = 1;              break;
      case IDX_USAGE:             usage             = 1;              break;
      case IDX_HEX_CHARSET:       hex_charset       = 1;              break;
      case IDX_COMBINATIONS:      combinations      = 1;              break;
      case IDX_SEQ_MAX:           seq_max           = atoi (optarg);  break;
      case IDX_OCCUR_MAX:         occur_max         = atoi (optarg);  break;
      case IDX_INCREMENT:         increment         = 1;
                                  increment_data    = optarg;         break;
      case IDX_START_AT:          start_at          = optarg;         break;
      case IDX_STOP_AT:           stop_at           = optarg;         break;
      case IDX_OUTPUT_FILE:       output_file       = optarg;         break;
      case IDX_CUSTOM_CHARSET_1:  custom_charset_1  = optarg;         break;
      case IDX_CUSTOM_CHARSET_2:  custom_charset_2  = optarg;         break;
      case IDX_CUSTOM_CHARSET_3:  custom_charset_3  = optarg;         break;
      case IDX_CUSTOM_CHARSET_4:  custom_charset_4  = optarg;         break;

      default: return (-1);
    }
  }

  if (usage)
  {
    usage_big_print (argv[0]);

    return (-1);
  }

  if (version)
  {
    printf ("v%4.02f\n", (double) VERSION_BIN / 100);

    return (-1);
  }

  if ((optind + 1) > argc)
  {
    usage_mini_print (argv[0]);

    return (-1);
  }

  if (seq_max == 1)
  {
    fprintf (stderr, "--seq-max must be set to at least 2\n");

    return (-1);
  }

  if (seq_max && start_at)
  {
    fprintf (stderr, "--seq-max can not be used with --start-at\n");

    return (-1);
  }

  if (seq_max && stop_at)
  {
    fprintf (stderr, "--seq-max can not be used with --stop-at\n");

    return (-1);
  }

  if (seq_max && combinations)
  {
    fprintf (stderr, "--seq-max can not be used with --combinations\n");

    return (-1);
  }

  if (occur_max == 1)
  {
    fprintf (stderr, "--occurrence-max must be set to at least 2\n");

    return (-1);
  }

  if (occur_max && start_at)
  {
    fprintf (stderr, "--occurrence-max can not be used with --start-at\n");

    return (-1);
  }

  if (occur_max && stop_at)
  {
    fprintf (stderr, "--occurrence-max can not be used with --stop-at\n");

    return (-1);
  }

  if (occur_max && combinations)
  {
    fprintf (stderr, "--occurrence-max can not be used with --combinations\n");

    return (-1);
  }

  /* buffers */

  char *mp_sys[6];

  mp_sys[0] = (char *) malloc (CHARSIZ); memset (mp_sys[0], 0, CHARSIZ);
  mp_sys[1] = (char *) malloc (CHARSIZ); memset (mp_sys[1], 0, CHARSIZ);
  mp_sys[2] = (char *) malloc (CHARSIZ); memset (mp_sys[2], 0, CHARSIZ);
  mp_sys[3] = (char *) malloc (CHARSIZ); memset (mp_sys[3], 0, CHARSIZ);
  mp_sys[4] = (char *) malloc (CHARSIZ); memset (mp_sys[4], 0, CHARSIZ);
  mp_sys[5] = (char *) malloc (CHARSIZ); memset (mp_sys[5], 0, CHARSIZ);

  char *mp_user[4];

  mp_user[0] = (char *) malloc (CHARSIZ); memset (mp_user[0], 0, CHARSIZ);
  mp_user[1] = (char *) malloc (CHARSIZ); memset (mp_user[1], 0, CHARSIZ);
  mp_user[2] = (char *) malloc (CHARSIZ); memset (mp_user[2], 0, CHARSIZ);
  mp_user[3] = (char *) malloc (CHARSIZ); memset (mp_user[3], 0, CHARSIZ);

  /* builtin charsets */

  char donec[CHARSIZ]; memset (donec, 0, CHARSIZ);

  int pos;
  int chr;

  for (pos = 0, chr =  'a'; chr <=  'z'; chr++) { donec[chr] = 1;
                                                  mp_sys[0][pos++] = chr; }

  for (pos = 0, chr =  'A'; chr <=  'Z'; chr++) { donec[chr] = 1;
                                                  mp_sys[1][pos++] = chr; }

  for (pos = 0, chr =  '0'; chr <=  '9'; chr++) { donec[chr] = 1;
                                                  mp_sys[2][pos++] = chr; }

  for (pos = 0, chr = 0x20; chr <= 0x7e; chr++) { if (donec[chr]) continue;
                                                  mp_sys[3][pos++] = chr; }

  for (pos = 0, chr = 0x20; chr <= 0x7e; chr++) { mp_sys[4][pos++] = chr; }

  for (pos = 0, chr = 0x00; chr <= 0xff; chr++) { mp_sys[5][pos++] = chr; }

  if (custom_charset_1) mp_expand (custom_charset_1, strlen (custom_charset_1), mp_user[0], mp_sys, hex_charset);
  if (custom_charset_2) mp_expand (custom_charset_2, strlen (custom_charset_2), mp_user[1], mp_sys, hex_charset);
  if (custom_charset_3) mp_expand (custom_charset_3, strlen (custom_charset_3), mp_user[2], mp_sys, hex_charset);
  if (custom_charset_4) mp_expand (custom_charset_4, strlen (custom_charset_4), mp_user[3], mp_sys, hex_charset);

  /* files in */

  #ifdef WINDOWS
  setmode (fileno (stdout), O_BINARY);
  setmode (fileno (stderr), O_BINARY);
  #endif

  FILE *fp_out = stdout;

  if (output_file)
  {
    if ((fp_out = fopen (output_file, "ab")) == NULL)
    {
      fprintf (stderr, "ERROR: %s: %s\n", output_file, strerror (errno));

      return (-1);
    }
  }

  setbuf (fp_out, NULL);

  /* load css */

  char *line_buf = argv[optind];

  size_t line_len = strlen (line_buf);

  cs_t css[PW_MAX];

  memset (css, 0, sizeof (css));

  int css_cnt = 0;

  size_t line_pos;

  for (line_pos = 0; line_pos < line_len; line_pos++)
  {
    char p0 = line_buf[line_pos];

    if (p0 == '?')
    {
      line_pos++;

      char p1 = line_buf[line_pos];

      switch (p1)
      {
        case 'l': add_cs_buf (mp_sys[0], css, css_cnt);
                  css_cnt++;
                  break;
        case 'u': add_cs_buf (mp_sys[1], css, css_cnt);
                  css_cnt++;
                  break;
        case 'd': add_cs_buf (mp_sys[2], css, css_cnt);
                  css_cnt++;
                  break;
        case 's': add_cs_buf (mp_sys[3], css, css_cnt);
                  css_cnt++;
                  break;
        case 'a': add_cs_buf (mp_sys[4], css, css_cnt);
                  css_cnt++;
                  break;
        case 'b': memcpy (css[css_cnt].cs_buf,  mp_sys[5], 256);
                  memset (css[css_cnt].cs_uniq,         1, 256);
                  css[css_cnt].cs_len = 256;
                  css_cnt++;
                  break;
        case '1': add_cs_buf (mp_user[0], css, css_cnt);
                  css_cnt++;
                  break;
        case '2': add_cs_buf (mp_user[1], css, css_cnt);
                  css_cnt++;
                  break;
        case '3': add_cs_buf (mp_user[2], css, css_cnt);
                  css_cnt++;
                  break;
        case '4': add_cs_buf (mp_user[3], css, css_cnt);
                  css_cnt++;
                  break;
        case '?': add_cs_buf ("?", css, css_cnt);
                  css_cnt++;
                  break;
        default:  fprintf (stderr, "ERROR: Syntax Error: %s\n", line_buf);
                  return (-1);
                  break;
      }
    }
    else
    {
      if (hex_charset)
      {
        line_pos++;

        if (line_pos == line_len) break;

        char p1 = line_buf[line_pos];

        char s[2] = { 0, 0 };

        s[0]  = hex_convert (p1) << 0;
        s[0] |= hex_convert (p0) << 4;

        add_cs_buf (s, css, css_cnt);
      }
      else
      {
        char s[2] = { p0, 0 };

        add_cs_buf (s, css, css_cnt);
      }

      css_cnt++;
    }
  }

  /* run css */

  if (start_at)
  {
    int start_at_len = strlen (start_at);

    if (start_at_len == css_cnt)
    {
      int i;

      for (i = 0; i < css_cnt; i++)
      {
        int pos = find_pos (&css[i], start_at[i]);

        if (pos == -1)
        {
          fprintf (stderr, "ERROR: value '%c' in position '%d' of start-at parameter '%s' is not part of position '%d' in mask '%s'\n", start_at[i], i + 1, start_at, i + 1, line_buf);

          return (-1);
        }
      }
    }
    else
    {
      fprintf (stderr, "ERROR: size of '%s' from start-at parameter is not equal to size of mask '%s'\n", start_at, line_buf);

      return (-1);
    }
  }

  if (stop_at)
  {
    int stop_at_len = strlen (stop_at);

    if (stop_at_len == css_cnt)
    {
      int i;

      for (i = 0; i < css_cnt; i++)
      {
        int pos = find_pos (&css[i], stop_at[i]);

        if (pos == -1)
        {
          fprintf (stderr, "ERROR: value '%c' in position '%d' of stop-at parameter '%s' is not part of position '%d' in mask '%s'\n", stop_at[i], i + 1, stop_at, i + 1, line_buf);

          return (-1);
        }
      }
    }
    else
    {
      fprintf (stderr, "ERROR: size of '%s' from stop-at parameter is not equal to size of mask '%s'\n", stop_at, line_buf);

      return (-1);
    }
  }

  int min = css_cnt;
  int max = css_cnt;

  if (increment)
  {
    char *s_min = strtok (increment_data, ":");
    char *s_max = strtok (NULL, ":");

    if (s_min == NULL)
    {
      fprintf (stderr, "ERROR: increment syntax error\n");

      return (-1);
    }

    if (s_max == NULL)
    {
      fprintf (stderr, "ERROR: increment syntax error\n");

      return (-1);
    }

    increment_min = atoi (s_min);
    increment_max = atoi (s_max);

    if (increment_min < min) min = increment_min;
    if (increment_max < max) max = increment_max;
  }

  /**
   * Calculate number of combinations and quit
   */

  if (combinations)
  {
    uint64_t cnt = 0;

    int len;

    for (len = min; len <= max; len++)
    {
      cnt += mp_get_sum (len, css);
    }

    #ifdef OSX
    printf ("%llu\n", (long long unsigned int) cnt);
    #endif

    #ifdef LINUX
    printf ("%llu\n", (long long unsigned int) cnt);
    #endif

    #ifdef WINDOWS
    printf ("%I64u\n", (long long unsigned int) cnt);
    #endif

    return (0);
  }

  int seq_start[PW_MAX];

  int len;

  for (len = 0; len < PW_MAX; len++)
  {
    int off = len - seq_max + 1;

    seq_start[len] = (off > 0) ? off : 0;
  }

  out_t *out = malloc (sizeof (out_t));

  for (len = min; len <= max; len++)
  {
    char word_buf[PW_MAX];

    #ifdef OSX
    word_buf[len] = '\n';

    int word_len = len + 1;
    #endif

    #ifdef LINUX
    word_buf[len] = '\n';

    int word_len = len + 1;
    #endif

    #ifdef WINDOWS
    word_buf[len + 0] = '\r';
    word_buf[len + 1] = '\n';

    int word_len = len + 1 + 1;
    #endif

    int occurs[256];

    memset (occurs, 0, sizeof (occurs));

    out->pos = 0;

    if (start_at)
    {
      int i;

      for (i = 0; i < len; i++)
      {
        css[i].cs_pos = find_pos (&css[i], start_at[i]);

        word_buf[i] = css[i].cs_buf[css[i].cs_pos];

        css[i].cs_pos++;
      }

      memcpy (out->buf + out->pos, word_buf, word_len);

      out->pos += word_len;

      start_at = NULL;
    }
    else
    {
      int i;

      for (i = 0; i < len; i++)
      {
        css[i].cs_pos = css[i].cs_len;
      }

      css[0].cs_pos = 0;
    }

    int first;

    while ((first = next (word_buf, css, len, occurs)) != -1)
    {
      if (seq_max)
      {
        int seq_cnt = 1;

        int i = seq_start[first];

        int max_seq_possible = len - i;

        if (max_seq_possible >= seq_max)
        {
          char prev = word_buf[i];

          for (i++; i < len; i++)
          {
            char cur = word_buf[i];

            if (cur == prev)
            {
              seq_cnt++;
            }
            else
            {
              seq_cnt = 1;

              prev = cur;
            }

            if (seq_cnt < seq_max) continue;

            for (i++; i < len; i++)
            {
              css[i].cs_pos = css[i].cs_len;
            }

            break;
          }
        }

        if (seq_cnt == seq_max) continue;
      }

      if (occur_max)
      {
        int i;

        for (i = 0; i < len; i++)
        {
          char c = word_buf[i];

          int occur_cnt = occurs[(int) c];

          if (occur_cnt >= occur_max) break;
        }

        if (i < len) continue;
      }

      memcpy (out->buf + out->pos, word_buf, word_len);

      out->pos += word_len;

      if (stop_at)
      {
        if (memcmp (word_buf + first, stop_at + first, len - first) == 0)
        {
          if (memcmp (word_buf, stop_at, len) == 0)
          {
            stop_at = NULL;

            break;
          }
        }
      }

      if (out->pos < OUTBUFSZ) continue;

      fwrite (out->buf, 1, out->pos, fp_out);

      out->pos = 0;
    }

    fwrite (out->buf, 1, out->pos, fp_out);

    out->pos = 0;
  }

  if (fp_out != stdout) fclose (fp_out);

  return 0;
}
