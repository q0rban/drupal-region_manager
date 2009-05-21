<?php
// $Id: region-manager-manage-form.tpl.php,v 1.1.2.3 2009/05/21 16:59:23 q0rban Exp $

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
 * - $data->state_select: Drop-down menu for assigning a state.
 * - $data->weight_select: Drop-down menu for setting weights.
 * - $data->operations: Row containing all the operations for the block.
 */
?>
<?php
  // Add table javascript.
  drupal_add_js('misc/tableheader.js');
  drupal_add_js(drupal_get_path('module', 'region_manager') .'/region_manager.js');
  drupal_add_js(array('block_name' => _region_manager_block_name()), 'setting');
  foreach ($block_states as $state => $title) {
    //drupal_add_tabledrag('blocks', 'match', 'sibling', 'block-region-select', 'block-region-'. $region, NULL, FALSE);
    drupal_add_tabledrag('region-manager-blocks-active_path', 'order', 'sibling', 'block-weight');
  }
?>

<?php foreach ($block_states as $state => $title): ?>
  <?php $row = 0; ?>
  <?php print $state_prefix[$state] ?>
  <div class="table-wrapper">
    <table id="region-manager-blocks-<?php print $state; ?>">
      <tbody>
        <tr class="rm-state rm-state-<?php print $state ?>">
          <td colspan="4" class="rm-state"><?php print $title ?></td>
        </tr>
        <tr class="rm-state-message rm-state-<?php print $state?>-message <?php print empty($block_listing[$state]) ? 'rm-state-empty' : 'rm-state-populated'; ?>">
          <td colspan="4"><em><?php print t('No @blocks @state', array('@block' => _region_manager_block_name(), '@state' => strtolower($title))); ?></em></td>
        </tr>
        <?php foreach ($block_listing[$state] as $delta => $data): ?>
        <tr class="rm-block rm-block-<?php print $delta; ?> rm-state-<?php print $state ?> <?php print $row % 2 == 0 ? 'odd' : 'even'; ?><?php print $data->row_class ? ' '. $data->row_class : ''; ?>">
          <td class="block"><?php print $data->block_title; ?></td>
          <td class="state"><?php print $data->state_select; ?></td>
          <td class="weight"><?php print $data->weight_select; ?></td>
          <td class="operations"><?php print $data->operations; ?></td>
        </tr>
        <?php $row++; ?>
        <?php endforeach; ?>
      </tbody>
    </table>
  </div>
  <?php print $state_suffix[$state] ?>
<?php endforeach; ?>

<?php print $form_submit; ?>
