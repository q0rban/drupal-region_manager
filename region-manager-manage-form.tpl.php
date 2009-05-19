<?php
// $Id: region-manager-manage-form.tpl.php,v 1.1.2.1 2009/05/18 01:38:26 q0rban Exp $

/**
 * @file block-admin-display-form.tpl.php
 * Default theme implementation to configure blocks.
 *
 * Available variables:
 * - $block_states: An array of states. Keyed by name with the title as value.
 * - $block_listing: An array of blocks keyed by region and then delta.
 * - $form_submit: Form submit button.
 *
 * Each $block_listing[$state] contains an array of blocks for that region.
 *
 * Each $data in $block_listing[$state] contains:
 * - $data->region_title: Region title for the listed block.
 * - $data->block_title: Block title.
 * - $data->region_select: Drop-down menu for assigning a region.
 * - $data->weight_select: Drop-down menu for setting weights.
 * - $data->throttle_check: Checkbox to enable throttling.
 * - $data->configure_link: Block configuration link.
 * - $data->delete_link: For deleting user added blocks.
 *
 * @see template_preprocess_block_admin_display_form()
 * @see theme_block_admin_display()
 */
?>
<?php
  // Add table javascript.
  drupal_add_js('misc/tableheader.js');
  drupal_add_js(drupal_get_path('module', 'region_manager') .'/region_manager.js');
  drupal_add_js(array('block_name' => _region_manager_block_name()), 'setting');
  foreach ($block_states as $state => $title) {
    //drupal_add_tabledrag('blocks', 'match', 'sibling', 'block-region-select', 'block-region-'. $region, NULL, FALSE);
    drupal_add_tabledrag('region-manager-blocks-active-path', 'order', 'sibling', 'block-weight', 'block-weight-'. RM_BLOCK_ACTIVE_PATH);
  }
?>
<table id="region-manager-blocks-active-path">
  <tbody>
    <?php $row = 0; ?>
    <tr class="rm-state rm-state-<?php print RM_BLOCK_ACTIVE_PATH ?>">
      <td colspan="4" class="rm-state"><?php print $block_states[RM_BLOCK_ACTIVE_PATH] ?></td>
    </tr>
    <tr class="rm-state-message rm-state-<?php print $state?>-message <?php print empty($block_listing[$state]) ? 'rm-state-empty' : 'rm-state-populated'; ?>">
      <td colspan="4"><em><?php print t('No @blocks', array('@block' => _region_manager_block_name())); ?></em></td>
    </tr>
    <?php foreach ($block_listing[RM_BLOCK_ACTIVE_PATH] as $delta => $data): ?>
    <tr class="rm-block rm-block-<?php print $delta; ?> rm-state-<?php print RM_BLOCK_ACTIVE_PATH ?> draggable <?php print $row % 2 == 0 ? 'odd' : 'even'; ?><?php print $data->row_class ? ' '. $data->row_class : ''; ?>">
      <td class="block"><?php print $data->block_title; ?></td>
      <td><?php print $data->region_select; ?></td>
      <td><?php print $data->weight_select; ?></td>
      <td class="operations"><?php print $data->operations; ?></td>
    </tr>
    <?php $row++; ?>
    <?php endforeach; ?>
    <?php unset($block_states[RM_BLOCK_ACTIVE_PATH]); ?>
  </tbody>
</table>

<table id="region-manager-blocks-available">
  <tbody>
    <?php $row = 0; ?>
    <?php foreach ($block_states as $state => $title): ?>
      <tr class="rm-state rm-state-<?php print $state ?>">
        <td colspan="4" class="rm-state"><?php print $title ?></td>
      </tr>
      <tr class="rm-state-message rm-state-<?php print $state?>-message <?php print empty($block_listing[$state]) ? 'rm-state-empty' : 'rm-state-populated'; ?>">
        <td colspan="4"><em><?php print t('No @blocks', array('@block' => _region_manager_block_name())); ?></em></td>
      </tr>
      <?php foreach ($block_listing[$state] as $delta => $data): ?>
      <tr class="rm-block rm-block-<?php print $delta; ?> rm-state-<?php print $state ?> <?php print $row % 2 == 0 ? 'odd' : 'even'; ?><?php print $data->row_class ? ' '. $data->row_class : ''; ?>">
        <td class="block"><?php print $data->block_title; ?></td>
        <td><?php print $data->region_select; ?></td>
        <td><?php print $data->weight_select; ?></td>
        <td class="operations"><?php print $data->operations; ?></td>
      </tr>
      <?php $row++; ?>
      <?php endforeach; ?>
    <?php endforeach; ?>
  </tbody>
</table>

<?php print $form_submit; ?>
