#ifndef TYPEDEF_H
#define TYPEDEF_H

#define ui8 unsigned char	
#define si8 signed char		
#define ui16 unsigned long	
#define si16 signed long		
#define ui32 unsigned int	
#define si32 signed int		
#ifdef __LINUX__
#define ui64 unsigned long long
#define si64 signed long long
#else if _WIN32_
#define ui64 unsigned __int64	
#define si64 signed __int64		
#endif
#endif