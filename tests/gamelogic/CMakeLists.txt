mu_add_test(
    NAME test_boid_steering
    SOURCES
        test_boid_steering.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_direction_turning
    SOURCES
        test_direction_turning.cpp
    LINK_LIBS
        MuClient
)

mu_add_test(
    NAME test_personal_shop_currency
    SOURCES
        test_personal_shop_currency.cpp
    LINK_LIBS
        MuClient
)
