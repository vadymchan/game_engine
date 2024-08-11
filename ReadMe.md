## Screenshots

![First Triangle Generalized RHI](screenshots/first_triangle_generalized_rhi.png)

## Dependencies

- LLVM + Clang Power Tools - for formatting and refactoring the code. Use `.clang-format`.

## naming conventions for this project:

For refactoring naming conventions this project uses **clang-format**.

| Code Element                     | Naming Convention                                 | Example                                  |
| -------------------------------- | ------------------------------------------------- | ---------------------------------------- |
| Classes                          | CamelCase                                         | `GameEngine`                             |
| Structures                       | CamelCase                                         | `Vector2D`                               |
| Unions                           | CamelCase                                         | `DataUnion`                              |
| Functions / Methods              | camelCase with `g_` prefix (for global functions) | `updatePosition()`, `g_initializeGame()` |
| Public Member Variables          | `m_` prefix + camelCase                           | `m_position`                             |
| Private Member Variables         | `m_` prefix + camelCase + `_` postfix             | `m_position_`                            |
| Protected Member Variables       | `m_` prefix + camelCase + `_` postfix             | `m_counter_`                             |
| Public Methods                   | camelCase                                         | `updatePosition()`                       |
| Protected Methods                | camelCase + `_` postfix                           | `run_()`                                 |
| Private Methods                  | camelCase + `_` postfix                           | `initialize_()`                          |
| Enums (both scoped and unscoped) | CamelCase                                         | `Color`                                  |
| Enum Constants                   | CamelCase                                         | `Difficulty::Easy`, `RED`                |
| Namespaces                       | lowercase with underscores                        | `game_logic`                             |
| Interface Classes                | `I` prefix + CamelCase                            | `ICollidable`                            |
| Template Parameters              | CamelCase                                         | `ContainerType`                          |
| Macros                           | UPPER_CASE_WITH_UNDERSCORES                       | `MAX_HEALTH`                             |
| Typedefs and Type Aliases        | CamelCase                                         | `BigInt`                                 |
| Static Constant Member Variables | `s_k` prefix + CamelCase                          | `s_kMaxValue`                            |
| Class Constant Member Variables  | `s_k` prefix + CamelCase                          | `s_kDefaultColor`                        |
| Constants                        | `k` prefix + CamelCase                            | `kMaxPlayers`                            |
| Static Variables                 | `s_` prefix + camelCase                           | `s_instanceCount`                        |
| Global Variables                 | `g_` prefix + camelCase                           | `g_gameState`                            |
| Global Constants                 | `g_k` prefix + CamelCase                          | `g_kInitialSpeed`                        |
| Class Members                    | `s_` prefix + camelCase                           | `s_memberVariable`                       |
| Class Methods                    | `s_` prefix + camelCase                           | `s_classMethod()`                        |
| Template Value                   | camelCase                                         | `defaultValue`                           |
| Type Template                    | CamelCase                                         | `TypeParam`                              |

P.S. for some elements i'm still not sure:

- for class methods do i really need to add `_s` prefix 🤔
- do i need to add `s_k`, `g_k` prefixes 🤔
