<?php

include_once 'includes/custom_menu.inc';

/**
 * Implements hook_preprocess_html().
 */
function touchm_preprocess_html(&$variables) {

  _touchm_add_css();

  // Cache path to theme for duration of this function:
  $path_to_theme = drupal_get_path('theme', 'touchm');
  // Add 'viewport' meta tag:
  drupal_add_html_head(
          array(
      '#tag' => 'meta',
      '#attributes' => array(
          'name' => 'viewport',
          'content' => 'width=device-width, initial-scale=1',
      ),
          ), 'proma:viewport_meta'
  );
}

function _touchm_add_css() {
  $theme_path = $theme_path = path_to_theme();
  $default_color = theme_get_setting('theme_color', 'touchm');
  drupal_add_css($theme_path . '/stylesheets/foundation.min.css');
  drupal_add_css($theme_path . '/stylesheets/style1.css');
  drupal_add_css($theme_path . '/stylesheets/' . $default_color . '.css');
  drupal_add_css($theme_path . '/plugins/titan/css/jquery.titanlighbox.css');
  drupal_add_css($theme_path . '/stylesheets/touchm.css');
}

function touchm_preprocess_page(&$vars) {

  $custom_main_menu = _custom_main_menu_render_superfish();
  if (!empty($custom_main_menu['content'])) {
    $vars['navigation'] = $custom_main_menu['content'];
  }


  if (arg(0) == 'node' && arg(1)) {
    $nid = arg(1);

    $node = node_load($nid);
    switch ($node->type) {
      case 'blog':
        $vars['title'] = t('Our blog');

        break;
    }
  }



  //search block form
  $seach_block_form = drupal_get_form('search_block_form');
  $seach_block_form['#id'] = 'searchform';

  $seach_block_form['search_block_form']['#prefix'] = '<div class="ten mobile-three columns">';
  $seach_block_form['search_block_form']['#suffix'] = '</div>';
  $seach_block_form['actions']['submit']['#prefix'] = '<div class="two mobile-one columns"><a id="search-block-button" class="button icon-search" href="javascript:$(\'#searchform\').submit();"></a>';
  $seach_block_form['actions']['submit']['#attributes']['class'][] = 'display-none';
  $seach_block_form['actions']['submit']['#suffix'] = '</div>';
  $seach_block_form['search_block_form']['#attributes']['placeholder'] = t('Enter search terms');
  $vars['seach_block_form'] = drupal_render($seach_block_form);
}

function touchm_breadcrumb($variables) {
  $breadcrumb = $variables['breadcrumb'];
  $output = '';
  if (!empty($breadcrumb)) {
    // Provide a navigational heading to give context for breadcrumb links to
    // screen-reader users. Make the heading invisible with .element-invisible.

    $br = '';
    if (!empty($breadcrumb)) {
      foreach ($breadcrumb as $b) {
        $br .= '<li>' . $b . '</li>';
      }
    }
    $output .= '<ul class="breadcrumbs">' . $br . '</ul>';
    return $output;
  }
}

function touchm_form_alter(&$form, &$form_state, $form_id) {
  global $user;
  if (!empty($form['actions']['submit'])) {
    $form['actions']['submit']['#attributes']['class'][] = 'medium button';
  }

  if ($form_id == 'simplenews_block_form_5') {

    $form['#prefix'] = '<div class="row collapse newsletter-input">';
    if ($user->uid) {
      if (isset($form['mail'])) {
        $form['mail']['#type'] = 'textfield';
        $mail_default_value =
                $form['mail']['#default_value'] = $form['mail']['#value'];
        $form['mail']['#default_value'] = $mail_default_value;
        unset($form['mail']['#value']);
      }
    }

    if (isset($form['mail'])) {

      $form['mail']['#prefix'] = '<div class="eight mobile-three columns">';
      $form['mail']['#suffix'] = '</div>';
    }

    if (isset($form['submit'])) {
      $form['submit']['#prefix'] = '<div class="four mobile-one columns">';
      $form['submit']['#suffix'] = '</div>';
      $form['submit']['#attributes']['class'] = array('button expand');
      $form['#suffix'] = '</div>';
    }
  }
}

function touchm_preprocess_node(&$variables) {
  $variables['view_mode'] = $variables['elements']['#view_mode'];
  // Provide a distinct $teaser boolean.
  $variables['teaser'] = $variables['view_mode'] == 'teaser';
  $variables['node'] = $variables['elements']['#node'];
  $node = $variables['node'];

  $variables['date'] = format_date($node->created);
  $variables['name'] = theme('username', array('account' => $node));

  $uri = entity_uri('node', $node);
  $variables['node_url'] = url($uri['path'], $uri['options']);
  $variables['title'] = check_plain($node->title);
  $variables['page'] = $variables['view_mode'] == 'full' && node_is_page($node);

  // Flatten the node object's member fields.
  $variables = array_merge((array) $node, $variables);

  // Helpful $content variable for templates.
  $variables += array('content' => array());
  foreach (element_children($variables['elements']) as $key) {
    $variables['content'][$key] = $variables['elements'][$key];
  }

  // Make the field variables available with the appropriate language.
  field_attach_preprocess('node', $node, $variables['content'], $variables);

  // Display post information only on certain node types.
  if (variable_get('node_submitted_' . $node->type, TRUE)) {
    $variables['display_submitted'] = TRUE;
    $date = format_date($node->created, 'custom', 'M d, Y');
    $node_url = url('node/' . $node->nid);
    $category = touchm_format_comma_field('field_tags', $node);
    $submitted = '<ul class="link-list">';
    $submitted .= '<li class="post-date"><span class="icon-calendar"></span> ' . l($date, 'node/' . $node->nid) . '</li>';
    $submitted .= '<li class="post-author"><span class="icon-user"></span>' . $variables['name'] . '</li>';
    $submitted .= '<li class="post-categories"><span class="icon-tags"></span><a href="' . $node_url . '">' . $category . '</a></li>';
    $submitted .= '<li class="post-comments"><span class="icon-comments"></span><a href="' . $node_url . '">' . $node->comment_count . ' ' . t('comments') . '</a></li>';


    $submitted .= '</ul>';

    $variables['submitted'] = $submitted; //t('Submitted by !username on !datetime', array('!username' => $variables['name'], '!datetime' => $variables['date']));
    $variables['user_picture'] = theme_get_setting('toggle_node_user_picture') ? theme('user_picture', array('account' => $node)) : '';
  } else {
    $variables['display_submitted'] = FALSE;
    $variables['submitted'] = '';
    $variables['user_picture'] = '';
  }

  // Gather node classes.
  $variables['classes_array'][] = drupal_html_class('node-' . $node->type);
  if ($variables['promote']) {
    $variables['classes_array'][] = 'node-promoted';
  }
  if ($variables['sticky']) {
    $variables['classes_array'][] = 'node-sticky';
  }
  if (!$variables['status']) {
    $variables['classes_array'][] = 'node-unpublished';
  }
  if ($variables['teaser']) {
    $variables['classes_array'][] = 'node-teaser';
  }
  if (isset($variables['preview'])) {
    $variables['classes_array'][] = 'node-preview';
  }

  // Clean up name so there are no underscores.
  $variables['theme_hook_suggestions'][] = 'node__' . $node->type;
  $variables['theme_hook_suggestions'][] = 'node__' . $node->nid;
}

function touchm_format_comma_field($field_category, $node, $limit = NULL) {
  $category_arr = array();
  $category = '';
  if (!empty($node->{$field_category}[LANGUAGE_NONE])) {
    foreach ($node->{$field_category}[LANGUAGE_NONE] as $item) {
      $term = taxonomy_term_load($item['tid']);
      if ($term) {
        $category_arr[] = l($term->name, 'taxonomy/term/' . $item['tid']);
      }

      if ($limit) {
        if (count($category_arr) == $limit) {
          $category = implode(', ', $category_arr);
          return $category;
        }
      }
    }
  }
  $category = implode(', ', $category_arr);

  return $category;
}

function touchm_field__field_image($variables) {
  $count = 0;
  if (!empty($variables['items'])) {
    $count = count($variables['items']);
  }
  $output = '';

  // Render the label, if it's not hidden.
  if (!$variables['label_hidden']) {
    $output .= '<div class="field-label"' . $variables['title_attributes'] . '>' . $variables['label'] . ':&nbsp;</div>';
  }

  // Render the items.

  $output .= '<div class="article_media">';
  if ($count > 1) {
    $output .= '<div class="simple-slider flexslider">';
  }

  $output .= '<ul class="slides">';
  foreach ($variables['items'] as $delta => $item) {
    $output.= '<li class="image-overlay">';
    $output .= drupal_render($item);
    $output .= '</li>';
  }
  $output .= '</ul>';
  if ($count > 1) {
    $output .= '</div>';
  }
  $output .= '</div>';

  // Render the top-level DIV.
  $output = '<div class="' . $variables['classes'] . '"' . $variables['attributes'] . '>' . $output . '</div>';

  return $output;
}

function touchm_tagadelic_weighted(array $vars) {
  $terms = $vars['terms'];
  $output = '';

  foreach ($terms as $term) {
    $output .= l($term->name, 'taxonomy/term/' . $term->tid, array(
                'attributes' => array(
                    'class' => array("tagadelic", "level" . $term->weight, 'small', 'button'),
                    'rel' => 'tag',
                    'title' => $term->description,
                )
                    )
            ) . " \n";
  }
  if (count($terms) >= variable_get('tagadelic_block_tags_' . $vars['voc']->vid, 12)) {
    $output .= theme('more_link', array('title' => t('more tags'), 'url' => "tagadelic/chunk/{$vars['voc']->vid}"));
  }
  return $output;
}

function touchm_pager($variables) {
  $tags = $variables['tags'];
  $element = $variables['element'];
  $parameters = $variables['parameters'];
  $quantity = $variables['quantity'];
  global $pager_page_array, $pager_total;

  // Calculate various markers within this pager piece:
  // Middle is used to "center" pages around the current page.
  $pager_middle = ceil($quantity / 2);
  // current is the page we are currently paged to
  $pager_current = $pager_page_array[$element] + 1;
  // first is the first page listed by this pager piece (re quantity)
  $pager_first = $pager_current - $pager_middle + 1;
  // last is the last page listed by this pager piece (re quantity)
  $pager_last = $pager_current + $quantity - $pager_middle;
  // max is the maximum page number
  $pager_max = $pager_total[$element];
  // End of marker calculations.
  // Prepare for generation loop.
  $i = $pager_first;
  if ($pager_last > $pager_max) {
    // Adjust "center" if at end of query.
    $i = $i + ($pager_max - $pager_last);
    $pager_last = $pager_max;
  }
  if ($i <= 0) {
    // Adjust "center" if at start of query.
    $pager_last = $pager_last + (1 - $i);
    $i = 1;
  }
  // End of generation loop preparation.

  $li_first = theme('pager_first', array('text' => (isset($tags[0]) ? $tags[0] : t('« first')), 'element' => $element, 'parameters' => $parameters));
  $li_previous = theme('pager_previous', array('text' => (isset($tags[1]) ? $tags[1] : t('‹ previous')), 'element' => $element, 'interval' => 1, 'parameters' => $parameters));
  $li_next = theme('pager_next', array('text' => (isset($tags[3]) ? $tags[3] : t('next ›')), 'element' => $element, 'interval' => 1, 'parameters' => $parameters));
  $li_last = theme('pager_last', array('text' => (isset($tags[4]) ? $tags[4] : t('last »')), 'element' => $element, 'parameters' => $parameters));

  if ($pager_total[$element] > 1) {
    if ($li_first) {
      $items[] = array(
          'class' => array('pager-first'),
          'data' => $li_first,
      );
    }
    if ($li_previous) {
      $items[] = array(
          'class' => array('pager-previous'),
          'data' => $li_previous,
      );
    }

    // When there is more than one page, create the pager list.
    if ($i != $pager_max) {
      if ($i > 1) {
        $items[] = array(
            'class' => array('pager-ellipsis'),
            'data' => '…',
        );
      }
      // Now generate the actual pager piece.
      for (; $i <= $pager_last && $i <= $pager_max; $i++) {
        if ($i < $pager_current) {
          $items[] = array(
              'class' => array('pager-item'),
              'data' => theme('pager_previous', array('text' => $i, 'element' => $element, 'interval' => ($pager_current - $i), 'parameters' => $parameters)),
          );
        }
        if ($i == $pager_current) {
          $items[] = array(
              'class' => array('pager-current', 'current'),
              'data' => '<a>' . $i . '</a>',
          );
        }
        if ($i > $pager_current) {
          $items[] = array(
              'class' => array('pager-item'),
              'data' => theme('pager_next', array('text' => $i, 'element' => $element, 'interval' => ($i - $pager_current), 'parameters' => $parameters)),
          );
        }
      }
      if ($i < $pager_max) {
        $items[] = array(
            'class' => array('pager-ellipsis'),
            'data' => '…',
        );
      }
    }
    // End generation.
    if ($li_next) {
      $items[] = array(
          'class' => array('pager-next'),
          'data' => $li_next,
      );
    }
    if ($li_last) {
      $items[] = array(
          'class' => array('pager-last'),
          'data' => $li_last,
      );
    }
    return '<h2 class="element-invisible">' . t('Pages') . '</h2>' . theme('item_list', array(
                'items' => $items,
                'attributes' => array('class' => array('pagination')),
            ));
  }
}

function touchm_field__field_portfolio_image($variables) {

  $output = '<section class="slider"><div class="gallery-slider flexslider"><ul class="slides">';

  // Render the label, if it's not hidden.
  if (!$variables['label_hidden']) {
    $output .= '<div class="field-label"' . $variables['title_attributes'] . '>' . $variables['label'] . ':&nbsp;</div>';
  }

  // Render the items.

  foreach ($variables['items'] as $delta => $item) {
    $classes = 'field-item ' . ($delta % 2 ? 'odd' : 'even');

    $image_url = '';
    if (!empty($item['#item']['uri'])) {
      $image_url = file_create_url($item['#item']['uri']);
    }
    $output .= '<li data-thumb="' . $image_url . '">' . drupal_render($item) . '</li>';
  }

  $output .= '</ul></div></div>';
  return $output;
}

