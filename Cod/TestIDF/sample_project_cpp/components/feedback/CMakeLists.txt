idf_component_register(
    SRCS "feedback.cpp"
    INCLUDE_DIRS "include"
    REQUIRES motors sensors wireless battery led
)