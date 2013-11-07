<?php

function touchm_form_system_theme_settings_alter(&$form, $form_state) {

  $form['settings'] = array(
      '#type' => 'vertical_tabs',
      '#title' => t('Theme settings'),
      '#weight' => 2,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );

  $form['settings']['home'] = array(
      '#type' => 'fieldset',
      '#title' => t('Homepage settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );
  $form['settings']['home']['home_tagline'] = array(
      '#type' => 'textarea',
      '#title' => t('Home tagline'),
      '#default_value' => theme_get_setting('home_tagline', 'touchm'),
  );

  $form['settings']['social_links'] = array(
      '#type' => 'fieldset',
      '#title' => t('Social links settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );

  $form['settings']['social_links']['twitter_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Twitter URL'),
      '#default_value' => theme_get_setting('twitter_url', 'touchm'),
  );
  $form['settings']['social_links']['facebook_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Facebook URL'),
      '#default_value' => theme_get_setting('facebook_url', 'touchm'),
  );
  $form['settings']['social_links']['google_plus_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Google+ URL'),
      '#default_value' => theme_get_setting('google_plus_url', 'touchm'),
  );
  $form['settings']['social_links']['linkedin_url'] = array(
      '#type' => 'textfield',
      '#title' => t('LinkedIn URL'),
      '#default_value' => theme_get_setting('linkedin', 'touchm'),
  );
  $form['settings']['social_links']['flickr_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Flickr URL'),
      '#default_value' => theme_get_setting('flickr_url', 'touchm'),
  );
  $form['settings']['social_links']['vimeo_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Vimeo URL'),
      '#default_value' => theme_get_setting('vimeo_url', 'touchm'),
  );
  $form['settings']['social_links']['rss_url'] = array(
      '#type' => 'textfield',
      '#title' => t('RSS URL'),
      '#default_value' => theme_get_setting('rss_url', 'touchm'),
  );

  $form['settings']['portfolio'] = array(
      '#type' => 'fieldset',
      '#title' => t('Portfolio settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );

  $form['settings']['portfolio']['default_portfolio'] = array(
      '#type' => 'select',
      '#title' => t('Default portfolio display'),
      '#options' => array(
          '2c' => 'Portfolio - 2cols',
          '3c' => 'Portfolio - 3cols',
          '4c' => 'portfolio - 4cols',
      ),
      '#default_value' => theme_get_setting('default_portfolio', 'touchm'),
  );
  $form['settings']['portfolio']['default_nodes_portfolio'] = array(
      '#type' => 'select',
      '#title' => t('Number nodes show on portfolio page'),
      '#options' => drupal_map_assoc(array(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 30)),
      '#default_value' => theme_get_setting('default_nodes_portfolio', 'touchm'),
  );


  $form['settings']['footer'] = array(
      '#type' => 'fieldset',
      '#title' => t('Footer settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );

  $form['settings']['footer']['footer_copyright_message'] = array(
      '#type' => 'textarea',
      '#title' => t('Footer copyright message'),
      '#default_value' => theme_get_setting('footer_copyright_message', 'touchm'),
  );

  $form['settings']['skin'] = array(
      '#type' => 'fieldset',
      '#title' => t('Skin settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
  );

  $color_arr = array(
      'color1' => t('Default'),
      'color2' => t('Color 2'),
      'color3' => t('Color 3'),
      'color4' => t('Color 4'),
      'color5' => t('Color 5'),
      'color6' => t('Color 6'),
      'color7' => t('Color 7'),
      'color8' => t('Color 8'),
  );

  $form['settings']['skin']['theme_color'] = array(
      '#type' => 'select',
      '#title' => t('Select default color'),
      '#default_value' => theme_get_setting('theme_color', 'touchm'),
      '#options' => $color_arr,
  );
}