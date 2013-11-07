<?php

/**
 * @file
 * Nodeimage theme functions.
 */
function theme_node_image_image_formatter($variables) {
  $item = $variables['item'];
  $entity_type = $variables['entity_type'];
  $entity = $variables['entity'];
  $field = $variables['field'];
  $settings = $variables['display_settings'];

  $image = array(
      'path' => $item['uri'],
      'alt' => $item['alt'],
      'title' => $item['title'],
      'style_name' => $settings['node_image_node_style'],
  );

  if (isset($item['width']) && isset($item['height'])) {
    $image['width'] = $item['width'];
    $image['height'] = $item['height'];
  }

  $entity_title = entity_label($entity_type, $entity);

  $caption = $entity_title;

  // Shorten the caption for the example styles or when caption shortening is active.
  $node_image_style = variable_get('node_image_style', 'default');
  $trim_length = variable_get('node_image_caption_trim_length', 75);
  if (((strpos($node_image_style, 'node_image/example') !== FALSE) || variable_get('node_image_caption_trim', 0)) && (drupal_strlen($caption) > $trim_length)) {
    $caption = drupal_substr($caption, 0, $trim_length - 5) . '...';
  }

  // Build the gallery id.
  list($id, $vid, $bundle) = entity_extract_ids($entity_type, $entity);
  $entity_id = !empty($id) ? $entity_type . '-' . $id : 'entity-id';


  if ($style_name = $settings['node_image_image_style']) {
    $path = image_style_url($style_name, $image['path']);
  } else {
    $path = file_create_url($image['path']);
  }
  $gallery_id = 'node_image_gallery-' . $entity_id;

  return theme('node_image_imagefield', array('image' => $image, 'path' => $path, 'title' => $caption, 'gid' => $gallery_id));
}

/**
 * Returns HTML for an image using a specific Colorbox image style.
 *
 * @param $variables
 *   An associative array containing:
 *   - image: image item as array.
 *   - path: The path of the image that should be displayed in the Colorbox.
 *   - title: The title text that will be used as a caption in the Colorbox.
 *   - gid: Gallery id for Colorbox image grouping.
 *
 * @ingroup themeable
 */
function theme_node_image_imagefield($variables) {
  
  $class = array('node_image', 'titan-lb');

  if ($variables['image']['style_name'] == 'hide') {
    $image = '';
    $class[] = 'js-hide';
  } elseif (!empty($variables['image']['style_name'])) {
    $image = theme('image_style', $variables['image']);
  } else {
    $image = theme('image', $variables['image']);
  }

  $options = array(
      'html' => TRUE,
      'attributes' => array(
          'title' => $variables['title'],
          'class' => implode(' ', $class),
          'rel' => $variables['gid'],
          'data-titan-group' => 'gallery'.$variables['gid'],
      )
  );

  return l('<span class="overlay-icon item-zoom"></span>' . $image, $variables['path'], $options);
}

/**
 * Preprocess variables for the node_image-insert-image.tpl.php file.
 */
function template_preprocess_node_image_insert_image(&$variables) {
  $class = array();
  $file = file_load($variables['item']['fid']);

  if (!empty($variables['widget']['settings']['insert_class'])) {
    $class = explode(' ', $variables['widget']['settings']['insert_class']);
  }
  $class[] = 'image-' . $variables['item']['style_name'];

  foreach ($class as $key => $value) {
    $class[$key] = drupal_html_class($value);
  }

  $variables['image_path'] = image_style_url($variables['item']['style_name'], $file->uri);

  if ($style_name = variable_get('node_image_image_style', '')) {
    $variables['link_path'] = image_style_url($style_name, $file->uri);
  } else {
    $variables['link_path'] = file_create_url($file->uri);
  }

  $variables['class'] = implode(' ', $class);

  $variables['gallery_id'] = '';
}
