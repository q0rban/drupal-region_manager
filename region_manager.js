// $Id: region_manager.js,v 1.1.2.4 2009/05/21 02:50:13 q0rban Exp $

/**
 * Show/hide form elements for editing a block title.
 */
Drupal.behaviors.blockTitle = function() {
  // Show hide the textfield depending on the radio states.
  $('input.region-manager-manage-form-title-status').change(function() {
    if (this.checked) {
      var textfield = $('input.form-text', $(this).parents('td.block'));
      switch (this.value) {
        // Default Title
        case '0':
          textfield.val('').hide('fast');
          break;

        // Disable Title
        case '1':
          textfield.val('<none>').hide();
          break;

        // Custom Title
        case '2':
          if (textfield.val() == '<none>') {
            textfield.val('');
          }
          textfield.show('fast');
          break;
      }
    }
  });

  // Instantiate a change so we can hide the title field if need be.
  $('input.region-manager-manage-form-title-status').change();
};

/**
 * Perform actions from operation links
 */
Drupal.behaviors.regionManagerOperations = function() {
  $('td.operations a').click(function() {
    var op = $(this).attr('class');
    var rowClass = $(this).parents('ul').attr('class');
    var row = $('tr.' + rowClass);
    var configWrapper = $('div.region-manager-block-title-wrapper', row);
    var tableDragObj = Drupal.tableDrag['region-manager-blocks-active_path'];
    var tables = $('#region-manager-manage-form table');

    switch(op) {
      case 'add':
        row.appendTo('table#region-manager-blocks-active_path').each(function() { tableDragObj.makeDraggable(this) });
        $('select.block-state-select', row).val('active_path');
        break;
        
      case 'configure':
        if (configWrapper.is(':hidden')) {
          configWrapper.show('fast');
        }
        else {
          configWrapper.hide('fast');
        }
        break;

      case 'remove':
        row.insertAfter('table#region-manager-blocks-active tr.rm-state-message');
        $('select.block-state-select', row).val('active');
        $('a.tabledrag-handle', row).remove();
        configWrapper.hide();
        break;

      case 'disable':
        row.insertAfter('table#region-manager-blocks-disabled tr.rm-state-message');
        $('select.block-state-select', row).val('disabled');
        $('a.tabledrag-handle', row).remove();
        configWrapper.hide();
        break;
    }
    if (op != 'configure') {
      tables.each(function() { checkEmptyRegions(this, row); });
    }
    $('tr', tables).filter(':odd').filter('.odd')
      .removeClass('odd').addClass('even')
    .end().end()
    .filter(':even').filter('.even')
      .removeClass('even').addClass('odd');

    return false;
  });

  var checkEmptyRegions = function(table, rowObject) {
    $('tr.rm-state-message', table).each(function() {
      // If the dragged row is in this region, but above the message row, swap it down one space.
      if ($(this).prev('tr').get(0) == rowObject.element) {
        // Prevent a recursion problem when using the keyboard to move rows up.
        if ((rowObject.method != 'keyboard' || rowObject.direction == 'down')) {
          rowObject.swap('after', this);
        }
      }
      // This region has become empty
      if ($(this).next('tr').is(':not(.draggable)') || $(this).next('tr').size() == 0) {
        $(this).removeClass('rm-state-populated').addClass('rm-state-empty');
      }
      // This region has become populated.
      else if ($(this).is('.rm-state-empty')) {
        $(this).removeClass('rm-state-empty').addClass('rm-state-populated');
      }
    });
  };
}

/**
 * Perform actions from operation links
 */
Drupal.behaviors.regionManagerNodeBlock = function() {
  $('a.region-manager-nodeblock-menu-link').click(function() {
    var typesList = $('div.region-manager-nodeblock-menu div.item-list');
    if (typesList.is(':hidden')) {
      typesList.show('fast');
    }
    else {
      typesList.hide('fast');
    }

    return false;
  });
}