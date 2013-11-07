<?php

/**
 * @file
 * Configuration options for lazyload Formatter settings
 */

/**
 * Menu callback; Settings administration.
 */
function lazyload_admin_settings() {

  $form['lazyload_image'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Lazy load all the images'),
    '#description'   => t('If enabled, all the images of the site will "lazy load"'),
    '#default_value' => variable_get('lazyload_image', 0),
  );
  $form['lazyload_image_style'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Lazy load all the image styles'),
    '#description'   => t('If enabled, all the image style of the site will "lazy load"'),
    '#default_value' => variable_get('lazyload_image_style', 0),
  );
  $form['lazyload_image_formatter'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Lazy load all the image formatters'),
    '#description'   => t('If enabled, all the image formatters of the site will "lazy load"'),
    '#default_value' => variable_get('lazyload_image_formatter', 0),
  );
  $form['lazyload_users'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Lazy load user images'),
    '#description'   => t('If enabled, user pictures will "lazy load"'),
    '#default_value' => variable_get('lazyload_users', 0),
  );

  // Page specific visibility configurations.
  $form['page_vis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pages'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['page_vis_settings']['lazyload_visibility_pages'] = array(
    '#type' => 'radios',
    '#title' => t('Activate lazy loading on specific pages'),
    '#options' => array(
       t('Every page except the listed pages'),
       t('The listed pages only'),
     ),
    '#default_value' => variable_get('lazyload_visibility_pages', 0),
  );
  $form['page_vis_settings']['lazyload_pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#title_display' => 'invisible',
    '#default_value' => variable_get('lazyload_pages', LAZYLOAD_PAGES),
    '#description' => t("Specify pages by using their paths. Enter one path per line. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>')),
    '#rows' => 10,
  );

  return system_settings_form($form);
}
