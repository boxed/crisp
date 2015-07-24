#pragma once

// Make the compiler a bit tougher on those warnings
// 4700: local variable 'name' used without having been initialized
// 4715: not all control paths return a value
// 4035: no return value
// 4517: access-declarations are deprecated, i.e. a protected/private member becomes public
// 4509: nonstandard extension used: 'function' uses SEH and 'object' has destructor
// 4513: destructor could not be generated
// 4800: performance warning on converting to bool. This is fired when you do: bool b = _TN("test"); so this is very important that it is an error
//
// Make the compiler a bit nicer on those warnings
// 4355: 'this' : used in base member initializer list
// 4786: identifier was truncated to 'number' characters in the debug information
#pragma warning (error: 4700 4715 4035 4517 4509 4513 4800 4172; disable: 4786 4355)

// 4702: unreachable code
//#pragma warning (error: 4702)

// removed from error list due to massive amounts of errors, among other things in MFC code
// 4511: copy constructor could not be generated
// 4512: assignment operator could not be generated
// 4510: default constructor could not be generated


// temporary disables
#pragma warning (disable: 4996 4018 4244 4146 4603 4731 4244)