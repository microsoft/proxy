
if(PROXY_BUILD_MODULES)
  target_sources(msft_proxy_tests
  PRIVATE
    main.cpp
  )
  target_sources(msft_proxy_tests
    PRIVATE
      FILE_SET CXX_MODULES FILES
      foo.cpp
      impl.cpp
  )
  target_link_libraries(msft_proxy_tests PRIVATE msft_proxy4::proxy_module)
endif()
