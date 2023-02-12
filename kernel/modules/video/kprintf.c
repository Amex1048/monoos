#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <config.h>
#include <string.h>
#include <devices/terminal.h>

#include <kprintf.h>

//flags

#define LEFT        (1U << 0)
#define PLUS        (1U << 1)
#define SPACE       (1U << 2)
#define HASH        (1U << 3)
#define ZEROPAD     (1U << 4)

#define SIGNED      (1U << 5)
#define UPPERCASE   (1U << 6)
#define PRECISION   (1U << 7)

// Macro

#define is_digit(n) ((n) >= '0' && (n) <= '9')

#define min(a, b) (((a) > (b)) ? (b) : (a))
#define max(a, b) (((a) > (b)) ? (a) : (b))

// Struct

enum length_type
{
    LENGTH_NONE = 0,
    LENGTH_HH = 1,
    LENGTH_H = 2,
    LENGTH_L = 3,
    LENGTH_LL = 4,
    LENGTH_J = 5,
    LENGTH_Z = 6,
    LENGTH_T = 7
};

static int32_t atoi(const char **s)
{
    int32_t num = 0;

    while(is_digit(**s))
    {
        num *= 10;
        num += **s - '0';

        ++*s;
    }

    return num;
}

static int32_t format_number(int32_t (put_char)(char), uint32_t number, uint32_t base, int32_t width, int32_t precision, uint32_t flags)
{
    int printed = 0;
    char sign = 0;
    char buf[32];
    int8_t dcount = 0; // Digits count

    const char *digits = "0123456789abcdef";
    if(flags & UPPERCASE) digits = "0123456789ABCDEF";

    // Copy of the number for processing
    uint32_t ncopy = number;

    // Defining sign if number is decimal
    if(base == 10)
    {
        if(flags & PLUS) sign = '+';
        if(flags & SPACE) sign = ' ';
        if((flags & SIGNED) && ((int32_t)number < 0))
        {
            sign = '-';
            ncopy = ~ncopy + 1;
        }
    }

    // Filling buffer with digits to print
    if(ncopy == 0) buf[0] = '0';
    while(ncopy > 0)
    {
        buf[dcount] = digits[ncopy % base];
        ncopy /= base;
        ++dcount;
    }

    // Calculating amount of spaces to print before number (or after if LEFT & (flags))
    if(sign != 0) --width;
    if((flags & HASH) && (number != 0))
    {
        switch (base)
        {
        case 8:
            --width;
            break;
        case 16:
            width -= 2;
            break;

        default:
            break;
        }
    }

    width -= max(precision, dcount);

    // Calculating amount of leading precision zeroes
    precision -= dcount;

    // Printing leading spaces if needed
    if(!(flags & LEFT) && !(flags & ZEROPAD))
    {
        while(width > 0)
        {
            put_char(' ');
            ++printed;
            --width;
        }
    }

    // Printing number sign and prefix
    if(sign != 0)
    {
        put_char(sign);
        ++printed;
    }
    if((flags & HASH) && (number != 0))
    {
        switch (base)
        {
        case 8:
            put_char('0');
            ++printed;
            break;
        case 16:
            put_char('0');
            put_char((flags & UPPERCASE) ? 'X' : 'x');
            printed += 2;
            break;

        default:
            break;
        }
    }

    // Printing leading zeropad zeroes
    if((flags & ZEROPAD) && !(flags & LEFT))
    {
        while(width > 0)
        {
            put_char('0');
            ++printed;
            --width;
        }
    }

    // Printing leading precision zeroes
    while(precision > 0)
    {
        put_char('0');
        ++printed;
        --precision;
    }

    // Printing digits
    while(dcount > 0)
    {
        --dcount;
        put_char(buf[dcount]);
        ++printed;
    }

    // Printing ending spaces if needed
    while(width > 0)
    {
        put_char(' ');
        ++printed;
        --width;
    }

    return printed;
}

static int32_t format_string(int32_t (put_char)(char), const char *s, int32_t width, int32_t precision, uint32_t flags)
{
    int32_t printed = 0;
    int32_t length = (int32_t)strlen(s);

    if(!(flags & PRECISION)) precision = INT32_MAX; // Precision was not specified

    width -= min(precision, length);

    // Printing leading spaces if needed
    if(!(flags & LEFT))
    {
        while (width > 0)
        {
            put_char(' ');
            ++printed;
            --width;
        }
    }

    while(*s && precision)
    {
        put_char(*s);
        ++printed;
        ++s;
        --precision;
    }

    // Printing ending spaces if needed
    while (width > 0)
    {
        put_char(' ');
        ++printed;
        --width;
    }

    return printed;
}

static int32_t format_char(int32_t (put_char)(char), char const c, int32_t width, uint32_t flags)
{
    --width;

    // Printing leading spaces if needed
    if(!(flags & LEFT))
    {
        while (width > 0)
        {
            put_char(' ');
            --width;
        }
    }

    put_char(c);

    // Printing ending spaces if needed
    while (width > 0)
    {
        put_char(' ');
        --width;
    }

    return 1;
}

static uint32_t get_argument(va_list *args, char const type, enum length_type length)
{
    uint32_t argument = 0;

    switch (type)
    {
    case 'i':
        switch (length)
        {
        case LENGTH_NONE:
            argument = (uint32_t)va_arg(*args, int);
            break;
        case LENGTH_HH:
            argument = (uint8_t)va_arg(*args, int);
            break;
        case LENGTH_H:
            argument = (uint16_t)va_arg(*args, int);
            break;
        case LENGTH_L:
            argument = (uint32_t)va_arg(*args, long int);
            break;
        case LENGTH_LL:
            argument = (uint32_t)va_arg(*args, long long int);
            break;
        case LENGTH_J:
            argument = (uint32_t)va_arg(*args, intmax_t);
            break;
        case LENGTH_Z:
            argument = (uint32_t)va_arg(*args, size_t);
            break;
        case LENGTH_T:
            argument = (uint32_t)va_arg(*args, ptrdiff_t);
            break;
        default:
            break;
        }
        break;
    case 'u':
    case 'x':
    case 'o':
        switch (length)
        {
        case LENGTH_NONE:
            argument = (uint32_t)va_arg(*args, unsigned int);
            break;
        case LENGTH_HH:
            argument = (uint8_t)va_arg(*args, unsigned int);
            break;
        case LENGTH_H:
            argument = (uint16_t)va_arg(*args, unsigned int);
            break;
        case LENGTH_L:
            argument = (uint32_t)va_arg(*args, unsigned long int);
            break;
        case LENGTH_LL:
            argument = (uint32_t)va_arg(*args, unsigned long long int);
            break;
        case LENGTH_J:
            argument = (uint32_t)va_arg(*args, uintmax_t);
            break;
        case LENGTH_Z:
            argument = (uint32_t)va_arg(*args, size_t);
            break;
        case LENGTH_T:
            argument = (uint32_t)va_arg(*args, ptrdiff_t);
            break;
        default:
            break;
        }
        break;
    case 'p':
        argument = (uint32_t)va_arg(*args, void *);
        break;
    case 'c':
        argument = (uint32_t)va_arg(*args, uint32_t);
        break;
    case 's':
        argument = (uint32_t)va_arg(*args, char *);
        break;
    default:
        break;
    }

    return argument;
}


// Main printf
int32_t vprintf(const char *format, va_list args, int32_t (put_char)(char))
{
    int32_t printed = 0;
    uint32_t flags;
    int32_t width;
    int32_t precision;
    enum length_type length;

    while(*format)
    {
        length = LENGTH_NONE;
        width = 0;
        precision = 1;
        flags = 0;

        if(*format != '%')
        {
            put_char(*format);
            ++printed;
            ++format;
            continue;
        }

        ++format;

        // Reading flags
        bool f = true;
        while (f)
        {
            switch (*format)
            {
            case '-':
                flags |= LEFT;
                ++format;
                break;
            case '+':
                flags |= PLUS;
                ++format;
                break;
            case ' ':
                flags |= SPACE;
                ++format;
                break;
            case '#':
                flags |= HASH;
                ++format;
                break;
            case '0':
                flags |= ZEROPAD;
                ++format;
                break;

            case '\0':
                goto end;

            default:
                f = false;
                break;
            }
        }

        // Reading width
        if(*format == '*')
        {
            width = va_arg(args, int32_t);
            ++format;
        }
        else
        {
            width = atoi(&format);
        }

        // Reading precision
        if(*format == '.')
        {
            flags &= ~ZEROPAD;  // Ignore zeropad flag when precision is given
            flags |= PRECISION; // Check if precision was given

            ++format;
            if(*format == '*')
            {
                precision = va_arg(args, int32_t);
                ++format;
            }
            else precision = atoi(&format);
        }

        // Reading length
        switch (*format)
        {
        case 'h':
            ++format;
            if(*format == 'h')
            {
                ++format;
                length = LENGTH_HH;
            }
            else length = LENGTH_H;
            if(!*format) goto end;
            break;
        case 'l':
            ++format;
            if(*format == 'l')
            {
                ++format;
                length = LENGTH_LL;
            }
            else length = LENGTH_L;
            if(!*format) goto end;
            break;
        case 'j':
            length = LENGTH_J;
            break;
        case 'z':
            length = LENGTH_Z;
            break;
        case 't':
            length = LENGTH_T;
            break;

        default:
            break;
        }

        uint32_t argument = 0;
        uint32_t *npointer;

        // Reading specifier
        switch (*format)
        {
        case 'i':
        case 'd':
            flags |= SIGNED;
            argument = get_argument(&args, 'i', length);
            printed += format_number(put_char, argument, 10, width, precision, flags);
            break;
        case 'u':
            argument = get_argument(&args, 'u', length);
            printed += format_number(put_char, argument, 10, width, precision, flags);
            break;

        case 'X':
            flags |= UPPERCASE;
            argument = get_argument(&args, 'x', length);
            printed += format_number(put_char, argument, 16, width, precision, flags);
            break;
        case 'x':
            argument = get_argument(&args, 'x', length);
            printed += format_number(put_char, argument, 16, width, precision, flags);
            break;

        case 'o':
            argument = get_argument(&args, 'o', length);
            printed += format_number(put_char, argument, 8, width, precision, flags);
            break;

        case 'p':
            argument = get_argument(&args, 'p', length);
            printed += format_number(put_char, argument, 16, width, precision, flags);
            break;

        case 'c':
            argument = get_argument(&args, 'c', length);
            printed += format_char(put_char, (const char)argument, width, flags);
            ++printed;
            break;

        case 's':
            argument = get_argument(&args, 's', length);
            printed += format_string(put_char, (const char *)argument, width, precision, flags);
            break;

        case 'n':
            switch (length)
            {
            case LENGTH_NONE:
                npointer = (uint32_t *)va_arg(args, int *);
                *(int *)npointer = (int)printed;
                break;
            case LENGTH_HH:
                npointer = (uint32_t *)va_arg(args, signed char *);
                *(signed char *)npointer = (signed char)printed;
                break;
            case LENGTH_H:
                npointer = (uint32_t *)va_arg(args, short int *);
                *(short int *)npointer = (short int)printed;
                break;
            case LENGTH_L:
                npointer = (uint32_t *)va_arg(args, long int *);
                *(long int *)npointer = (long int)printed;
                break;
            case LENGTH_LL:
                npointer = (uint32_t *)va_arg(args, long long int *);
                *(long long int *)npointer = (long long int)printed;
                break;
            case LENGTH_J:
                npointer = (uint32_t *)va_arg(args, intmax_t *);
                *(intmax_t *)npointer = (intmax_t)printed;
                break;
            case LENGTH_Z:
                npointer = (uint32_t *)va_arg(args, size_t *);
                *(size_t *)npointer = (size_t)printed;
                break;
            case LENGTH_T:
                npointer = (uint32_t *)va_arg(args, ptrdiff_t *);
                *(ptrdiff_t *)npointer = (ptrdiff_t)printed;
                break;
            default:
                break;
            }
            break;

        case '%':
            put_char('%');
            ++printed;
            break;

        default:
            if(!*format) goto end;
            break;
        }

        ++format;
    }

end:
    return printed;
}

// Printf variations

// Basic printf, prints to the terminal
int32_t kprintf(const char *format, ...)
{
    int32_t printed;
    va_list args;

    va_start(args, format);
    printed = vprintf(format, args, print_char);
    va_end(args);

    return printed;
}

#ifdef DEBUG
#include <devices/com.h>
// Debug printf
#ifdef DEBUG_COM
int32_t dbg_kprintf(const char *format, ...)
{
    int32_t printed;
    va_list args;

    va_start(args, format);
    printed = vprintf(format, args, com_send_char);
    va_end(args);

    return printed;
}
#endif
#endif
