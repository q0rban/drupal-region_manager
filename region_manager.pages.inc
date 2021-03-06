<?php
// $Id: region_manager.pages.inc,v 1.2 2010/02/09 00:13:20 q0rban Exp $

/**
 * Define our states.
 */

// Block is active for the path.
define('RM_BLOCK_ACTIVE_PATH', 'active_path');
// Block is a content block and only visible on the page from which it was created.
define('RM_BLOCK_ACTIVE_CONTENT', 'active_content');
// Block is active elsewhere.
define('RM_BLOCK_ACTIVE', 'active');
// Block is disabled.
define('RM_BLOCK_DISABLED', 'disabled');

/**
 * @file
 *   Page callbacks for managing regions.
 */

/**
 * Menu callback to prepare the manage form for viewing.
 *
 * @param $theme_key
 *   A string containing the key of the active theme.
 * @param $region
 *   A string containing the region to be managed.
 * @return
 *   The HTML output of the page.
 */
function region_manager_manage($key, $region) {
  global $theme_key;

  $output = '';

  // Fetch and sort blocks
  $blocks = region_manager_blocks_load_all($key, $region);

  module_load_include('inc', 'block', 'block.admin');

  // If we're setting blocks for a theme different than the active theme,
  // we need to temporarily override the global $theme_key var for
  // _block_compare to work properly.
  if ($theme_key != $key) {
    $orig_theme = $theme_key;
    $theme_key = $key;
  }

  usort($blocks, '_block_compare');

  // Now reset the $theme_key back if it was changed.
  if ($orig_theme) {
    $theme_key = $orig_theme;
  }

  $output .= drupal_get_form('region_manager_manage_form', $blocks, $theme_key, $region);

  return $output;
}

/**
 * Generate main blocks administration form.
 *
 * @param $form_state
 *   A keyed array containing the current state of the form.
 * @param $blocks
 *   An array of active and available blocks for this region.
 * @param $theme_key
 *   A string containing the key of the active theme.
 * @param $region
 *   A string containing the region to be managed.
 * @return
 *   The form structure.
 */
function region_manager_manage_form(&$form_state, $blocks, $theme_key, $region) {
  // Add CSS
  drupal_add_css(drupal_get_path('module', 'block') .'/block.css', 'module', 'all', FALSE);

  // Build form tree
  $form = array(
    // '#action' => arg(4) ? url('admin/build/block/list/'. $theme_key) : url('admin/build/block'),
    '#tree' => TRUE,
  );

  $block_states = array();

  // If we're coming from a page, set that up as the top row label.
  if ($path = $_REQUEST['destination']) {
    $block_states[RM_BLOCK_ACTIVE_PATH] = t('Active on @path', array('@path' => _region_manager_path_name($path)));
  }

  // Now add the Active and Disabled row labels.
  $block_states[RM_BLOCK_ACTIVE] = t('Active elsewhere');
  $block_states[RM_BLOCK_DISABLED] = t('Disabled');

  $form['#block_states'] = $block_states;
  $form['#region_manager'] = compact('theme_key', 'region', 'path');

  // Weights range from -delta to +delta, so delta should be at least half
  // of the amount of blocks present. This makes sure all blocks in the same
  // region get an unique weight.
  $weight_delta = round(count($blocks) / 2);

  // Prepare the form for each block.
  foreach ($blocks as $i => $block) {
    $key = $block['module'] .'_'. $block['delta'];

    $form[$key]['theme'] = array(
      '#type' => 'hidden',
      '#value' => $theme_key
    );
    $form[$key]['module'] = array(
      '#type' => 'value',
      '#value' => $block['module'],
    );
    $form[$key]['delta'] = array(
      '#type' => 'value',
      '#value' => $block['delta'],
    );

    // Block administration name.
    $form[$key]['info'] = array(
      '#value' => $block['info'], 
    );

    // Block operations
    $form[$key]['operations'] = array(
      '#type' => 'markup',
      '#value' => _region_manager_block_operations($block, $key),
    );

    // Set up the radios for the title status.
    $form[$key]['title_status'] = array(
      '#attributes' => array('class' => 'region-manager-manage-form-title-status'),
      '#type' => 'radios',
      '#default_value' => _region_manager_block_title_status($block['title']),
      '#options' => array(t('Use Default Title'), t('Disable Title'), t('Use Custom Title')),
    );

    // Block title
    $form[$key]['title'] = array(
      '#attributes' => array('class' => 'region-manager-manage-form-title'),
      '#type' => 'textfield',
      '#size' => 40,
      '#default_value' => $block['title'],
    );

    // If this is a content block, we need to add a checkbox so the user can expose
    // the block to other pages.
    if ($block['visibility'] == -1) {
      $form[$key]['expose'] = array(
        '#type' => 'checkbox',
        '#title' => 'Expose',
        '#default_value' => FALSE,
        '#description' => t('Expose this content @block to other pages.  <strong>NOTE: this cannot be undone.</strong>',  array('@block' => _region_manager_block_name())),
      );
    }

    // Block weight
    $form[$key]['weight'] = array(
      '#type' => 'weight',
      '#default_value' => $block['weight'],
      '#delta' => $weight_delta,
    );

    $form[$key]['state'] = array(
      '#type' => 'select',
      '#default_value' => _region_manager_block_state($block, $path),
      '#options' => $block_states,
    );
    $form[$key]['visibility'] = array(
      '#type' => 'value',
      '#value' => $block['visibility'],
    );
    $form[$key]['pages'] = array(
      '#type' => 'value',
      '#value' => $block['pages'],
    );

    $form[$key]['#attributes']['class'] = 'draggable';

    if ($block['status']) {
      // We can't allow removal of items if the block visibility is 0 (active on all 
      // pages) or 2 (php code).
      if ($block['visibility'] == 0 || $block['visibility'] == 2) {
        $form[$key]['#attributes']['class'] .= ' locked';
        $form[$key]['info']['#value'] .= ' '. t('(Locked)');
        $form[$key]['state']['#default_value'] = RM_BLOCK_ACTIVE_PATH;
      }
    }
    // Otherwise we flag this block as being disabled, to aid in processing the form 
    // after submission.
    else {
      $form[$key]['disabled'] = array('#type' => 'hidden', '#value' => TRUE);
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save @blocks', array('@block' => _region_manager_block_name())),
  );

  return $form;
}

/**
 * Process main blocks administration form submission.
 */
function region_manager_manage_form_submit($form, &$form_state) {
  $theme_key = $form['#region_manager']['theme_key'];
  $region    = $form['#region_manager']['region'];
  $path      = $form['#region_manager']['path'];

  $frontpage = variable_get('site_frontpage', 'node');
  $path = ($path == $frontpage) ? '<front>' : $path;

  foreach ($form_state['values'] as $key => $block) {
    // Clean up the title.
    $block['title'] = trim($block['title']);

    // Now let's go through our different states.
    switch($block['state']) {
      case RM_BLOCK_ACTIVE_PATH:
        $block['region'] = $region;
        $block['status'] = 1;

        // If the block was originally disabled, we need to set it up for our usage.
        if ($block['disabled']) {
          $block['visibility'] = 1;
          $block['pages'] = $path;
        }

        // If this is a content block and 'expose' was checked, change visibility to 1.
        if ($block['expose']) {
          $block['visibility'] = 1;
        }

        // Otherwise, all we need to do is add our path to pages.
        elseif (($block['visibility'] == 1) && !preg_match('/^'. preg_quote($path, '/') .'$/im', $block['pages'])) {
          $block['pages'] .= empty($block['pages']) ? $path : "\n". $path;
        }
        break;

      case RM_BLOCK_ACTIVE:
        $block['region'] = $region;
        $block['status'] = 1;

        // Remove path from the pages.
        $block['pages'] = trim(preg_replace('/^'. preg_quote($path, '/') .'$/im', '', $block['pages']));

        // Now check to see if pages is empty, if so, completely disable.
        if ($block['visibility'] == 1 && empty($block['pages'])) {
          unset($block['disabled']);
          $block['status'] = 0;
          $block['region'] = '';
        }
        break;

      case RM_BLOCK_DISABLED:
        $block['region'] = '';
        if ($block['visibility'] == 1) {
          $block['pages'] = '';
        }
        break;
    }

    db_query("UPDATE {blocks} SET status = %d, weight = %d, region = '%s', visibility = %d, pages = '%s', title = '%s' WHERE module = '%s' AND delta = '%s' AND theme = '%s'", $block['status'], $block['weight'], $block['region'], $block['visibility'], $block['pages'], $block['title'], $block['module'], $block['delta'], $block['theme']);
  }

  drupal_set_message(t('The @block settings have been updated.', array('@block' => _region_manager_block_name())));
  cache_clear_all();
}

/**
 * Process variables for region-manager-manage-form.tpl.php.
 *
 * The $vars array contains the following arguments:
 * - $form
 *
 * @see region-manager-manage-form.tpl.php
 * @see theme_region_manager_manage_form()
 */
function template_preprocess_region_manager_manage_form(&$vars) {
  $theme_key = $vars['form']['#region_manager']['theme_key'];

  $block_states = $vars['block_states'] = $vars['form']['#block_states'];
  $vars['state_prefix'] = $vars['state_suffix'] = array();

  foreach ($block_states as $state => $value) {
    _region_manager_manage_form_state_wrappers($vars, $state);
    $vars['block_listing'][$state] = array();
  }

  // Initialize disabled blocks array.
  $vars['block_listing'][BLOCK_REGION_NONE] = array();

  // Set up to track previous region in loop.
  $last_state = '';
  foreach (element_children($vars['form']) as $i) {
    $block = &$vars['form'][$i];

    // Only take form elements that are blocks.
    if (isset($block['info'])) {
      // Fetch state for current block.
      $state = $block['state']['#default_value'];

      // Set special classes needed for table drag and drop.
      $vars['form'][$i]['state']['#attributes']['class'] = 'block-state-select block-state-'. $state;
      $vars['form'][$i]['weight']['#attributes']['class'] = 'block-weight block-weight-'. $state;

      $vars['block_listing'][$state][$i]->row_class = isset($block['#attributes']['class']) ? $block['#attributes']['class'] : '';
      $vars['block_listing'][$state][$i]->block_modified = isset($block['#attributes']['class']) && strpos($block['#attributes']['class'], 'block-modified') !== FALSE ? TRUE : FALSE;

      $vars['block_listing'][$state][$i]->block_config = '<strong class="block-info">'. drupal_render($block['info']) .'</strong>';
      $vars['block_listing'][$state][$i]->block_config .= '<div class="region-manager-block-config-wrapper">';
      $vars['block_listing'][$state][$i]->block_config .= drupal_render($block['title_status']) . drupal_render($block['title']);
      $vars['block_listing'][$state][$i]->block_config .= ($block['expose']) ? drupal_render($block['expose']) : '';
      $vars['block_listing'][$state][$i]->block_config .= '</div>';

      $vars['block_listing'][$state][$i]->operations = drupal_render($block['operations']);

      $vars['block_listing'][$state][$i]->state_select = drupal_render($block['state']) . drupal_render($block['theme']);
      $vars['block_listing'][$state][$i]->weight_select = drupal_render($block['weight']);

      $last_state = $state;
    }
  }

  $vars['form_submit'] = drupal_render($vars['form']);
}

/**
 * Helper function to assemble the prefixes and suffixes for the state tables.
 */
function _region_manager_manage_form_state_wrappers(&$vars, $state) {
  $theme_key = $vars['form']['#region_manager']['theme_key'];
  $region    = $vars['form']['#region_manager']['region'];
  $vars['state_prefix'][$state] = $vars['state_suffix'][$state] = '';

  switch($state) {
    case RM_BLOCK_ACTIVE_PATH:
      $vars['state_prefix'][$state] = '<div id="region-manager-active-path-wrapper">';
      // Output the create links, if there are any.
      if ($links = region_manager_create_links($theme_key, $region)) {
        $vars['state_prefix'][$state] .= theme('region_manager_create_menu', $links);
      }
      break;

    case RM_BLOCK_ACTIVE_CONTENT:
      $vars['state_prefix'][$state] = '</div><div id="region-manager-active-content-wrapper">';
      break;

    case RM_BLOCK_ACTIVE:
      $vars['state_prefix'][$state] = '</div><div id="region-manager-active-disabled-tables-wrapper">';
      break;

    case RM_BLOCK_DISABLED:
      $vars['state_suffix'][$state] = '</div>';
  }

  return $output;
}

/**
 * Helper function to assemble a list of operations for a block.
 */
function _region_manager_block_operations($block, $key) {
  $links = array();

  // Set up all the operations.
  $args = array('@block' => _region_manager_block_name('upper'));
  $ops = array(
    'disable' => t('Disable @block completely', $args),
    'remove' => t('Remove @block from this page', $args),
    'configure' => t('Configure @block', $args),
    'add' => t('Add @block to this page', $args),
  );

  // Get the path and destination.
  $path = 'admin/build/block/configure/'. $block['module'] .'/'. $block['delta'];
  $dest = drupal_get_destination();

  foreach($ops as $op => $title) {
    $options = array(
      'attributes' => array(
        'title' => $title,
        'class' => $op,
      ),
      'query' => $dest,
    );
  
    $links[] = array('data' => l($title, $path, $options), 'class' => 'rm-block-'. $op .'-operation');
  }

  return theme('item_list', $links, NULL, 'ul', array('class' => 'rm-block-'. $key));
}

/**
 * Helper function to determine the state of a block.
 *
 * @return
 *   A string signifying the block state.
 */
function _region_manager_block_state($block, $path) {
  // Block is disabled and available for use.
  if ($block['status'] == 0) {
    return RM_BLOCK_DISABLED;
  }
  // Block is active on this path.
  if (_region_manager_block_check_visibility($block, $path)) {
    return RM_BLOCK_ACTIVE_PATH;
  }
  // Block is a content block and only visible on the page from which it was created.
  if ($block['visibility'] == -1) {
    return RM_BLOCK_ACTIVE_CONTENT;
  }
  // Block is active elsewhere.
  return RM_BLOCK_ACTIVE;
}

/**
 * Theme function to output a list of nodeblock content types to add.
 *
 * @param $links
 *   An array containing the available content types to add.
 * @return
 *   The HTML list of links.
 */
function theme_region_manager_create_menu($links) {
  $output = '<div class="region-manager-create-menu">';

  // The arguments array for the t() func.
  $t_args = array('@block' => _region_manager_block_name());

  // Link options.  We set a title here since we're going to be using
  // a fancy button.
  $options = array('attributes' => array(
    'title' => t('Create and add a new content @block', $t_args),
    'class' => 'region-manager-create-menu-link',
  ));
  $output .= '<h4>'. l(t('Create content @block', $t_args), 'node/add', $options) .'</h4>';

  $output .= '<div class="item-list-wrapper"><p>'. t('Click on a link below to create a new content @block.', $t_args) .'</p>';
  $output .= theme('item_list', $links);
  $output .= '</div></div>';

  return $output;
}

/**
 * Helper function to determine the status of a title.
 *
 * Title states:
 *   0 = Using the default block title.
 *   1 = The title is disabled.
 *   2 = A custom title is being used.
 *
 * @param $title
 *   A string containing the current title.
 * @return
 *   An integer containing the title state.
 */
function _region_manager_block_title_status($title) {
  $title_status = 0; // Default title.

  if ($title) {
    $title_status++; // Disabled title.
    if ($title != '<none>') {
      $title_status++; // Custom title.
    }
  }

  return $title_status;
}