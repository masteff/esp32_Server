idf_component_register(SRCS "main.c" "./drivers/wifi_driver.c" "./drivers/http_server.c" "./drivers/uart_driver.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "./webpage/index.html" "./webpage/styles.css" "./webpage/app.js" "./webpage/favicon.ico")
