#ifndef MONOKL__LOGGING_H
#define MONOKL__LOGGING_H

#include <SDL2/SDL_log.h>

#define log_verbose(...) SDL_LogVerbose(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
#define log_debug(...) SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
#define log_message(...) SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_INFO, __VA_ARGS__)
#define log(...) log_message(__VA_ARGS__)
#define log_info(...) SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
#define log_warn(...) SDL_LogWarn(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
#define log_error(...) SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
#define log_critical(...) SDL_LogCritical(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)

#endif
