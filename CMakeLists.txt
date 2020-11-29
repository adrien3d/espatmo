set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components)

# Add only the display srcs you are going to use if you want to keep compiled srcs to a minimum:
set(srcs 
    "espatmo.cpp"
)

# If the project does not use a touch display component FT6X36-IDF can be removed or #commented
idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES nvs_flash esp-tls
)
