# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c"
                    "esp_hid_gap.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

{% comment %} 
set(include_dirs "/include") {% endcomment %}

register_component()




