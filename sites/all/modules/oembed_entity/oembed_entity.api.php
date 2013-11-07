<?php
/**
 * @file
 * Hooks provided by the oEmbed Entity module.
 */

/**
 * Define a configuration form for a block.
 *
 * For info about viewer and access callbacks:
 * @see oembed_entity_default_viewer()
 * @see oembed_entity_default_access()
 *
 * @param Array $entity_handlers
 *   A map of all entities and their bundles, with callback functions
 *   specified.
 */
function hook_oembed_entity_handlers_alter(&$entity_handlers) {
  $entity_handlers['file']['image']['viewer'] = 'mymodule_oembed_entity_file_image_viewer';
  // Make all files use this access callback.
  if (isset($entity_handlers['file']) && is_array($entity_handlers['file'])) {
    foreach ($entity_handlers['file'] as $bundle_id => $bundle) {
      $entity_handlers['file'][$bundle_id]['access'] = 'mymodule_oembed_entity_file_access';
    }
  }
}
