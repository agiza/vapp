<section id="options">
  <?php
  $terms = array();
  $vid = NULL;
  $vid_machine_name = 'portfolio_categories';
  $vocabulary = taxonomy_vocabulary_machine_name_load($vid_machine_name);
  if (!empty($vocabulary->vid)) {
    $vid = $vocabulary->vid;
  }
  if (!empty($vid)) {
    $terms = taxonomy_get_tree($vid);
  }
  ?>
  <ul id="filters" class="option-set right" data-option-key="filter">
    <li><a href="#filter" data-option-value="*" class="button small selected"><?php print t('Show All'); ?></a></li>
    <?php if (!empty($terms)): ?>
      <?php foreach ($terms as $term): ?>
        <li><a href="#filter" data-option-value=".tid-<?php print $term->tid; ?>" class="button small"><?php print $term->name; ?></a></li>
      <?php endforeach; ?>
    <?php endif; ?>
  </ul>

</section>