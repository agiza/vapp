<?php
/**
 * @file
 * Provide an fractionslider style plugin for Views. This file is autoloaded by views.
 */

/**
  * Implementation of views_plugin_style().
  */
class views_fractionslider_style_plugin extends views_plugin_style {
  /**
   * Set default options
   */
function option_definition() {
  $options = parent::option_definition();
  $options['dataspeed'] = array('default' => 1000);
  $options['class'] = array('default' => '');
  $options['datadelay'] = array('default' => 1000);
  $options['space'] = array('default' => 30);
  $options['pager'] = array('default' => 'false');
  $options['controls'] = array('default' => 'false');

    return $options;
  }

  /**
   * Render the given style.
   */
function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['dataspeed'] = array(
      '#title' => t('Data Speed'),
      '#description' => t('Time in ms the in transition takes. Default is 1000.'),
      '#type' => 'textfield',
      '#size' => '10',
      '#default_value' => $this->options['dataspeed'],
    );
    $form['datadelay'] = array(
      '#title' => t('Data Delay'),
      '#description' => t('The time in ms before the in transition starts. Default is 1000.'),
      '#type' => 'textfield',
      '#size' => '10',
      '#default_value' => $this->options['datadelay'],
    );
    $form['space'] = array(
      '#title' => t('Field Spacing'),
      '#description' => t('The Space/Gap between fields while sliding. Similar Paddin-Top. Default is 30'),
      '#type' => 'textfield',
      '#size' => '10',
      '#default_value' => $this->options['space'],
    );
    $form['class'] = array(
      '#title' => t('Div class'),
      '#description' => t('The class to provide on the Div element itself.'),
      '#type' => 'textfield',
      '#size' => '30',
      '#default_value' => $this->options['class'],
    );
    $form['pager'] = array(
       '#type' => 'select',
       '#title' => t('Pager'),
       '#options' => array(
          'true' => t('True'),
         'false' => t('False'),
       ),
       '#default_value' => $this->options['pager'],
       '#description' => t('Set Pager True or False. Default is True.'),
    );
    $form['controls'] = array(
       '#type' => 'select',
       '#title' => t('Controls'),
       '#options' => array(
          'true' => t('True'),
         'false' => t('False'),
       ),
       '#default_value' => $this->options['controls'],
       '#description' => t('Set Controls True or False. Default is True.'),
   );
  }
}
