<?php
/**
 * @file
 * slider_type.features.inc
 */

/**
 * Implements hook_node_info().
 */
function slider_type_node_info() {
  $items = array(
    'slider' => array(
      'name' => t('Slider'),
      'base' => 'node_content',
      'description' => t('Slider'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}
