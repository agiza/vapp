oEmbed Entity module

Makes all system entities available to the oEmbed provider.

Usage:
Install module and go to:
http://example.com/oembed/endpoint?url=http://example.com/user/123
to retrieve the oEmbed code for an entity with ID 123 and type user.

As a third URL argument, the desired view mode can be specified. This view
mode is passed on to any default or custom viewer functions.

Permissions:
Make sure to set the proper permissions, allowing the access to the desired
entity types for the various roles.

Customization:
It is possible to define custom callback functions to handle both access and
viewing of the entities by using the hook_oembed_entity_handlers_alter() and
implementing callbacks with the same structure as the provided default
functions that handles access and viewing. 
Notice that different entity types behave different, and therefore, the default
functions might be unable to handle certain entity types. In this case, you
need to implement your own viewer and/or access callbacks in a custom module.

NOTE:
By default, this module will overrule all other URLs registered by the
oembedprovider module, which means that node/123 will also hit the handlers of
oembed_entity. 

To prevent this, set a drupal variable named
"oembed_entity_aggresive_path_disable" with the value of TRUE.

The entities will then still be available as by using URLs like 
http://example.com/oembed/endpoint?url=http://example.com/entity/user/123
To keep the URL references from oEmbed consumers working, this module
automatically redirects any requests to /entity/TYPE/ID to its corresponding
drupal path.