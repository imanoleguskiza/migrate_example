<?php

/**
 * @file
 * Contains IntegrationTestArticlesMigration class.
 */

/**
 * Class EuropaEuCommonPageMigration .
 */
class EuropaEuCommonPageMigration extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('common_page', $arguments);

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationNode::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationNode('common_page', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('title_field', 'title');
    $this->addFieldMapping('body', 'body');
    $this->addFieldMapping('field_see_also_items', 'see_also_item')->sourceMigration('EuropaEuMigrateSeeAlso');
    $this->addFieldMapping('field_downloads_items', 'download_item')->sourceMigration('EuropaEuMigrateDownloads');
    $this->addFieldMapping('field_video_gallery_items', 'video_gallery_item')->sourceMigration('EuropaEuMigrateVideoGallery');

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');

    $this->addFieldMapping('promote')->defaultValue(FALSE);
    $this->addFieldMapping('status')->defaultValue(NODE_PUBLISHED);
  }

  /**
   * Implements prepareRow().
   */
  public function prepareRow(&$row) {
    // Process the body field.
    $row->body = _europaeu_migrate_import_contents_filter($row->body, '');
  }

  /**
   * Implements prepare().
   */
  public function prepare(&$entity, $row) {
    $alias = trim(_europaeu_migrate_path($row->original_url), '/');
    $entity->path['pathauto'] = 0;
    if (isset($entity->nid)) {
      $path = array(
        'source' => "node/$entity->nid",
        'alias' => $alias,
        'language' => $entity->language,
      );
      path_save($path);
      unset($entity->path['alias']);
    }
    else {
      $entity->path['alias'] = $alias;
    }
  }


  /**
   * Implements complete().
   */
  public function complete($entity, \stdClass $source_row) {
    parent::complete($entity, $source_row);
    $alias = drupal_get_path_alias('node/' . $entity->nid);
    if (!empty($alias)) {
      $menu_info = db_select('menu_links', 'ml')
        ->condition('ml.link_path', $alias)
        ->condition('ml.menu_name', 'main-menu')
        ->fields('ml', array('mlid'))
        ->execute()
        ->fetchAll();
      if (count($menu_info) > 0) {
        foreach ($menu_info as $key => $value) {
          $menu_link_item = menu_link_load($menu_info[$key]->mlid);
          $menu_link_item['link_path'] = 'node/' . $entity->nid;
          menu_link_save($menu_link_item);
        }
      }
    }
  }

}
