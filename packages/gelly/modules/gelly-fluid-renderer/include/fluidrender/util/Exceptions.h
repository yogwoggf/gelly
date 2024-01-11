#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

// Helper macros for tossing exceptions

#define THROW_IF_FALSY(expr, msg)                       \
	if (!(expr)) {                                      \
		throw std::runtime_error(__FUNCSIG__ ": " msg); \
	}

#endif	// EXCEPTIONS_H