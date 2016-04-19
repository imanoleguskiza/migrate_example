<?php
/**
 * @file
 * Code for the reps migrate menu feature.
 */

/**
 * Class EuropaEuMenuAbstractMigration.
 */
abstract class EuropaEuMenuAbstractMigration extends EuropaEuAbstractMigration {

  /**
   * Constructor of the migrate class for Menus.
   *
   * @param string $migration_key
   *   Key identifying the folder containing the datas to migrate.
   * @param array $arguments
   *   Properties of the migration class.
   */
  public function __construct($migration_key, $arguments = array()) {
    parent::__construct($migration_key, $arguments);

    $this->addFieldMapping('link_path', 'link');
    $this->addFieldMapping('link_title', 'title');
    $this->addFieldMapping('weight', 'weight');
    $this->addFieldMapping('customized', 'customized')->defaultValue(1);
    $this->addFieldMapping('language', 'default_language')->defaultValue('und');
  }

  /**
   * Set Translation for menu item.
   */
  public function complete($item, \stdClass $source_row) {
    $this->menuTranslations($item, $source_row);

    // Update a link to be external.
    db_update('menu_links')
      ->fields(array(
        'external' => 1,
      ))
      ->condition('mlid', $item->mlid)
      ->execute();
  }

  /**
   * Save field translations for the specified menu item.
   *
   * @param \stdClass $item
   *   Item to translate.
   * @param \stdClass $source_row
   *   Source info for the translation.
   */
  public function menuTranslations(\stdClass $item, \stdClass $source_row) {
    foreach ($source_row->getAvailableLanguages() as $language) {
      if ($language != $source_row->getDefaultLanguage()) {
        $source_row->setSourceValues($language);
        $mlid = $item->mlid;
        $translation = $source_row->title;
        $name = 'menu:item:' . $mlid . ':title';
        i18n_string_translation_update($name, $translation, $language);
      }
    }
  }

}

/**
 * Class EuropaEuServicesMigration.
 */
class EuropaEuServicesMigration extends EuropaEuMenuAbstractMigration {

  /**
   * Constructor of the migrate class for services menu.
   *
   * @param array $arguments
   *   Properties of the migration class.
   */
  public function __construct($arguments = array()) {
    parent::__construct('menu_services', $arguments);

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationMenuLinks::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationMenuLinks('menu-service-tools'));
    $this->addFieldMapping('menu_name')->defaultValue('menu-service-tools');
  }

}

/**
 * Class EuropaEuMainMenuMigration.
 */
class EuropaEuMainMenuMigration extends EuropaEuMenuAbstractMigration {

  /**
   * Constructor of the migrate class for services menu.
   *
   * @param array $arguments
   *   Properties of the migration class.
   */
  public function __construct($arguments = array()) {
    parent::__construct('main_menu', $arguments);

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationMenuLinks::getKeySchema()
    ));

    $this->setDestination(new MigrateDestinationMenuLinks('main-menu'));
    $this->addFieldMapping('menu_name')->defaultValue('main-menu');
    $this->addFieldMapping('plid', 'parent')
      ->sourceMigration(array('EuropaEuMigrateMainMenu'));
  }

  /**
   * Overrides prepareRow() method.
   *
   * @param object $row
   *   Current row to change.
   *
   * @return bool
   *   Pass or skip a row.
   */
  public function prepareRow(&$row) {
    // Always start your prepareRow implementation with this clause. You need to
    // be sure your parent classes have their chance at the row, and that if
    // they return FALSE (indicating the row should be skipped) you pass that
    // on.
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    if ($row->level > 3) {
      return FALSE;
    }

    if ($row->link == 'empty') {
      $row->link = 'nolink';
    }

    return TRUE;
  }

  /**
   * Create Stub.
   */
  protected function createStub(Migration $migration, array $source_id) {
    $item = array(
      'link_path' => '<front>',
      'link_title' => t('Stub for @id', array('@id' => $source_id[0])),
      'menu_name' => 'main-menu',
      'weight' => 0,
      'expanded' => 0,
    );

    // On saving, the $item is passed by reference and hence
    // is given other properties, including the item ID.
    $item_id = menu_link_save($item);
    if ($item_id) {
      return array($item_id);
    }
    else {
      return FALSE;
    }
  }

}
