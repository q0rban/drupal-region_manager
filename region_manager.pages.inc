<?php
// $Id$

/**
 * @file
 *   Page callbacks for managing regions.
 */

/**
 * Menu callback to prepare the manage form for viewing.
 */
function region_manager_manage($theme_key, $region = NULL) {
  // Fetch and sort blocks
  $blocks = region_manager_blocks_load_all($theme_key, $region);

  module_load_include('inc', 'block', 'block.admin');
  usort($blocks, '_block_compare');

  return drupal_get_form('region_manager_manage_form', $blocks, $theme_key, $region);
}

/**
 * Generate main blocks administration form.
 */
function region_manager_manage_form(&$form_state, $blocks, $theme_key, $region) {
  // Add CSS
  drupal_add_css(drupal_get_path('module', 'block') .'/block.css', 'module', 'all', FALSE);

  // Build form tree
  $form = array(
    // '#action' => arg(4) ? url('admin/build/block/list/'. $theme_key) : url('admin/build/block'),
    '#tree' => TRUE,
  );

  $block_regions = array();

  // If we're coming from a page, set that up as the top row label.
  if ($path = $_REQUEST['destination']) {
    $frontpage = variable_get('site_frontpage', 'node');
    $alias = drupal_get_path_alias($path);
    if ($path == $frontpage) {
      $path = 'frontpage';
      $alias = t('home page');
    }
    $block_regions[$path] = t('Active on %path', array('%path' => $alias));
  }

  // Now add the Active and Disabled row labels.
  $block_regions[$region] = t('Active');
  $block_regions[BLOCK_REGION_NONE] = t('Disabled');

  $form['#block_regions'] = $block_regions;
  $form['#region_manager'] = array(
    'theme_key' => $theme_key,
    'region' => $region,
    'path' => $path,
  );

  // Weights range from -delta to +delta, so delta should be at least half
  // of the amount of blocks present. This makes sure all blocks in the same
  // region get an unique weight.
  $weight_delta = round(count($blocks) / 2);

  foreach ($blocks as $i => $block) {
    $key = $block['module'] .'_'. $block['delta'];
    $form[$key]['module'] = array(
      '#type' => 'value',
      '#value' => $block['module'],
    );
    $form[$key]['delta'] = array(
      '#type' => 'value',
      '#value' => $block['delta'],
    );
    $form[$key]['info'] = array(
      '#value' => check_plain($block['info'])
    );
    $form[$key]['theme'] = array(
      '#type' => 'hidden',
      '#value' => $theme_key
    );
    $form[$key]['weight'] = array(
      '#type' => 'weight',
      '#default_value' => $block['weight'],
      '#delta' => $weight_delta,
    );
    $form[$key]['region'] = array(
      '#type' => 'select',
      '#default_value' => $block['region'],
      '#options' => $block_regions,
    );
    // Now we customize this block depending on its status.
    if (_region_manager_block_check_visibility($block, $path)) {
      $block['region'] = $path;
    }
    $form[$key]['#attributes']['class'] = 'draggable';
    if ($block['visibility'] != 1) {
      if ($block['status']) {
        $form[$key]['#attributes']['class'] = 'locked';
        $form[$key]['info']['#value'] .= ' '. t('(Locked)');
      }
      else {
        $form[$key]['disabled'] = array('#type' => 'hidden', '#value' => TRUE);
      }
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save @blocks', array('@block' => strtolower(variable_get('region_manager_block_name', 'Block')))),
  );

  return $form;
}

/**
 * Process main blocks administration form submission.
 */
function region_manager_manage_form_submit($form, &$form_state) {
  extract($form['#region_manager']);

  foreach ($form_state['values'] as $block) {
    // Set the block status.
    $block['status'] = $block['region'] != BLOCK_REGION_NONE;

    // Now let's go through our different scenarios for enabled blocks.
    if ($block['status']) {
      // If the block is enabled for this path, we have to set the region and
      // add the path to $block['pages'].
      if ($path && ($block['region'] == $path)) {
        // Set the region.
        $block['region'] = $region;
        // Add the path to pages, if it's not already there.
        if (!preg_match('/^'. $path .'$/im', $block['pages'])) {
          $block['pages'] .= empty($block['pages']) ? $path : "\n". $path;
        }
      }
      // Otherwise, if it's active, but not enabled for this path, make sure 
      // we remove the path from $block['pages']
      elseif ($block['region'] == $region) {
        // Remove path from the pages.
        if (_region_manager_block_check_visibility($block, $path)) {
          $block['pages'] = preg_replace('/^'. $path .'$/im', '', $block['pages']);
        }
      }

      // If the block was originally disabled, we need to set it up for our usage.
      if ($block['disabled']) {
        $block['visibility'] = 1;
        $block['pages'] = $path;
      }
    }
    // Block is disabled, not much to do.
    else {
      // Clean up the pages setting.
      if ($block['visibility'] == 1) {
        $block['pages'] = '';
      }
    }
      ob_start();
      print_r($block);
      drupal_set_message(ob_get_contents());
      ob_end_clean();
    //db_query("UPDATE {blocks} SET status = %d, weight = %d, region = '%s', throttle = %d WHERE module = '%s' AND delta = '%s' AND theme = '%s'", $block['status'], $block['weight'], $block['region'], isset($block['throttle']) ? $block['throttle'] : 0, $block['module'], $block['delta'], $block['theme']);
  }
  drupal_set_message(t('The block settings have been updated.'));
  cache_clear_all();
}

/**
 * Process variables for block-admin-display.tpl.php.
 *
 * The $vars array contains the following arguments:
 * - $form
 *
 * @see block-admin-display.tpl.php
 * @see theme_block_admin_display()
 */
function template_preprocess_region_manager_manage_form(&$vars) {
  global $theme_key;

  $block_regions = $vars['form']['#block_regions'];
  $vars['block_regions'] = $block_regions + array(BLOCK_REGION_NONE => t('Disabled'));

  $vars['block_name'] = variable_get('region_manager_block_name', 'Block');

  foreach ($block_regions as $key => $value) {
    // Highlight regions on page to provide visual reference.
    drupal_set_content($key, '<div class="block-region">'. $value .'</div>');
    // Initialize an empty array for the region.
    $vars['block_listing'][$key] = array();
  }

  // Initialize disabled blocks array.
  $vars['block_listing'][BLOCK_REGION_NONE] = array();

  // Set up to track previous region in loop.
  $last_region = '';
  foreach (element_children($vars['form']) as $i) {
    $block = &$vars['form'][$i];

    // Only take form elements that are blocks.
    if (isset($block['info'])) {
      // Fetch region for current block.
      $region = $block['region']['#default_value'];

      // Set special classes needed for table drag and drop.
      $vars['form'][$i]['region']['#attributes']['class'] = 'block-region-select block-region-'. $region;
      $vars['form'][$i]['weight']['#attributes']['class'] = 'block-weight block-weight-'. $region;

      $vars['block_listing'][$region][$i]->row_class = isset($block['#attributes']['class']) ? $block['#attributes']['class'] : '';
      $vars['block_listing'][$region][$i]->block_modified = isset($block['#attributes']['class']) && strpos($block['#attributes']['class'], 'block-modified') !== FALSE ? TRUE : FALSE;
      $vars['block_listing'][$region][$i]->block_title =  drupal_render($block['info']);
      $vars['block_listing'][$region][$i]->region_select = drupal_render($block['region']) . drupal_render($block['theme']);
      $vars['block_listing'][$region][$i]->weight_select = drupal_render($block['weight']);
      $vars['block_listing'][$region][$i]->throttle_check = $vars['throttle'] ? drupal_render($block['throttle']) : '';
      $vars['block_listing'][$region][$i]->configure_link = drupal_render($block['configure']);
      $vars['block_listing'][$region][$i]->delete_link = !empty($block['delete']) ? drupal_render($block['delete']) : '';
      $vars['block_listing'][$region][$i]->printed = FALSE;

      $last_region = $region;
    }
  }

  $vars['form_submit'] = drupal_render($vars['form']);
}

/**
 * Helper function to restructure the output from region_manager_blocks_load_all,
 * to just have all blocks in one array, rather than structured by [$region]['active'], etc.
 */
function _region_manager_blocks_restructure($blocks) {
  $output = array();
  foreach($blocks as $status) {
    foreach($status as $state) {
      foreach($state as $block) {
        $output[] = $block;
      }
    }
  }
  return $output;
}