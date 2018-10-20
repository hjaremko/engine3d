#define _USE_MATH_DEFINES
#define WINDOW_HEIGHT 400
#define WINDOW_WIDTH  800

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

float crossProduct( const sf::Vector2f& a, const sf::Vector2f& b )
{
    return a.x * b.y - a.y * b.x;
}

sf::Vector2f intersect( sf::Vector2f a, sf::Vector2f b, sf::Vector2f c, sf::Vector2f d )
{
    sf::Vector2f tmp( crossProduct( a, b ), crossProduct( c, d ) );

    float det = crossProduct( sf::Vector2f( a.x - b.x, a.y - b.y ), sf::Vector2f( c.x - d.x, c.y - d.y ) );

    tmp.x = crossProduct( sf::Vector2f( tmp.x, a.x - b.x ), sf::Vector2f( tmp.y , c.x - d.x ) ) / det;
    tmp.y = crossProduct( sf::Vector2f( tmp.x, a.y - b.y ), sf::Vector2f( tmp.y , c.y - d.y ) ) / det;

    return tmp;
}

class Player : public sf::Drawable
{
public:
    Player() {}
    Player( sf::Vector2f t_start )
    {
        m_position = t_start;
    }

    void draw( sf::RenderTarget& t_target, sf::RenderStates t_states ) const override
    {
        sf::Vertex line[] =
        {
            sf::Vertex( m_position ),
            sf::Vertex( sf::Vector2f( cos( m_angle ) * 15 + m_position.x, sin( m_angle ) * 15 + m_position.y ) )
        };

        sf::CircleShape circle;
        circle.setRadius( 1 );
        circle.setPosition( m_position );

        t_target.draw( circle, t_states );
        t_target.draw( line, 2, sf::Lines, t_states );
    }

    void moveBy( float t_x, float t_y )
    {
        m_position.x += t_x;
        m_position.y += t_y;
    }

    void changeAngleBy( float t_value )
    {
        m_angle += t_value;
    }

    sf::Vector2f getPosition() const
    {
        return m_position;
    }

    float getAngle() const
    {
        return m_angle;
    }

    void moveForward()
    {
        moveBy( cos( m_angle ), sin( m_angle ) );
    }

    void moveBackward()
    {
        moveBy( -cos( m_angle ), -sin( m_angle ) );
    }

    void moveLeft()
    {
        moveBy( cos( m_angle - ( M_PI / 2 ) ), sin( m_angle - ( M_PI / 2 ) ) );
    }

    void moveRight()
    {
        moveBy( cos( m_angle + ( M_PI / 2 ) ), sin( m_angle + ( M_PI / 2 ) ) );
    }

    sf::Vector2f m_position;
    float        m_angle{ 0.0 };
};

class Wall3d : public sf::Drawable
{
public:
    float qx1 = 0.0;
    float y1a = 0.0;
    float qx2 = 0.0;
    float y2a = 0.0;

    float y1b = 0.0;
    float y2b = 0.0;

    sf::Vector2f topStart;
    sf::Vector2f topEnd;
    sf::Vector2f bottomStart;
    sf::Vector2f bottomEnd;

    void adjust()
    {
        topStart.x    = topStart.x    + WINDOW_WIDTH  / 2;
        topStart.y    = topStart.y    + WINDOW_HEIGHT / 2;
        topEnd.x      = topEnd.x      + WINDOW_WIDTH  / 2;
        topEnd.y      = topEnd.y      + WINDOW_HEIGHT / 2;

        bottomStart.x = topStart.x;
        bottomStart.y = bottomStart.y + WINDOW_HEIGHT / 2;
        bottomEnd.x   = topEnd.x;
        bottomEnd.y   = bottomEnd.y   + WINDOW_HEIGHT / 2;
    }

    void draw( sf::RenderTarget& t_target, sf::RenderStates t_states ) const override
    {
        sf::Vertex top[] =
        {
            sf::Vertex( topStart ),
            sf::Vertex( topEnd )
        };

        sf::Vertex bottom[] =
        {
            sf::Vertex( bottomStart ),
            sf::Vertex( bottomEnd )
        };

        sf::Vertex left[] =
        {
            sf::Vertex( bottomStart ),
            sf::Vertex( topStart )
        };

        sf::Vertex right[] =
        {
            sf::Vertex( bottomEnd ),
            sf::Vertex( topEnd )
        };

        t_target.draw( top,    2, sf::Lines, t_states );
        t_target.draw( bottom, 2, sf::Lines, t_states );
        t_target.draw( left,   2, sf::Lines, t_states );
        t_target.draw( right,  2, sf::Lines, t_states );
    }
};

class Wall : public sf::Drawable
{
public:
    Wall() {}
    Wall( sf::Vector2f t_start, sf::Vector2f t_end )
    {
        m_start = t_start;
        m_end = t_end;
    }

    sf::Vector2f m_start;
    sf::Vector2f m_end;

    void draw( sf::RenderTarget& t_target, sf::RenderStates t_states ) const override
    {
        sf::Vertex line[] =
        {
            sf::Vertex( m_start ),
            sf::Vertex( m_end )
        };

        t_target.draw( line, 2, sf::Lines, t_states );
    }

    Wall relativeTransform( Player& t_player )
    {
        Wall tmp;

        sf::Vector2f transformedStart = m_start - t_player.getPosition();
        sf::Vector2f transformedEnd   = m_end   - t_player.getPosition();
        
        sf::Vector2f relativeTmp( transformedStart.x * cos( t_player.getAngle() ) + transformedStart.y * sin( t_player.getAngle() ),
                                  transformedEnd.x   * cos( t_player.getAngle() ) + transformedEnd.y   * sin( t_player.getAngle() ) );

        transformedStart.x = transformedStart.x * sin( t_player.getAngle() ) - transformedStart.y * cos( t_player.getAngle() );
        transformedEnd.x   = transformedEnd.x   * sin( t_player.getAngle() ) - transformedEnd.y   * cos( t_player.getAngle() );

        tmp.m_start.x = transformedStart.x;
        tmp.m_start.y = relativeTmp.x;
        tmp.m_end.x   = transformedEnd.x;
        tmp.m_end.y   = relativeTmp.y;

        return tmp;
    }

    static Wall3d perspectiveTransform( Wall t_relativeWall )
    {
        Wall3d tmp;

        sf::Vector2f perspectiveStart;
        sf::Vector2f perspectiveEnd;
        
        if ( t_relativeWall.m_start.y > 0 || t_relativeWall.m_end.y > 0 )
        {
            perspectiveStart = intersect( t_relativeWall.m_start, t_relativeWall.m_end, sf::Vector2f( -0.0001,  0.0001 ), sf::Vector2f( -20.0, 5.0 ) );
            perspectiveEnd   = intersect( t_relativeWall.m_start, t_relativeWall.m_end, sf::Vector2f(  0.0001, -0.0001 ), sf::Vector2f(  20.0, 5.0 ) );

            if ( t_relativeWall.m_start.y <= 0 )
            {
                if ( perspectiveStart.y > 0 )
                {
                    t_relativeWall.m_start.x = perspectiveStart.x;
                    t_relativeWall.m_start.y = perspectiveStart.y;
                }
                else
                {
                    t_relativeWall.m_start.x = perspectiveEnd.x;
                    t_relativeWall.m_start.y = perspectiveEnd.y;
                }
            }

            if ( t_relativeWall.m_end.y <= 0 )
            {
                if ( perspectiveStart.y > 0 )
                {
                    t_relativeWall.m_end.x = perspectiveStart.x;;
                    t_relativeWall.m_end.y = perspectiveStart.y;
                }
                else
                {
                    t_relativeWall.m_end.x = perspectiveEnd.x;
                    t_relativeWall.m_end.y = perspectiveEnd.y;
                }
            }

            tmp.topStart.x    = -t_relativeWall.m_start.x * 64 / t_relativeWall.m_start.y;
            tmp.topStart.y    = -350 / t_relativeWall.m_start.y;
            tmp.bottomStart.y =  350 / t_relativeWall.m_start.y;

            tmp.topEnd.x      = -t_relativeWall.m_end.x * 64 / t_relativeWall.m_end.y;
            tmp.topEnd.y      = -350 / t_relativeWall.m_end.y;
            tmp.bottomEnd.y   =  350 / t_relativeWall.m_end.y;

            tmp.adjust();
        }

        return tmp;
    }

};




int main( int argc, char const *argv[] )
{
    sf::RenderWindow window( sf::VideoMode( WINDOW_WIDTH, WINDOW_HEIGHT ), "3D Engine Test" );

    Wall leftWall(  sf::Vector2f( 5.0, 5.0 ), sf::Vector2f( 5.0, 50.0 ) );
    Wall rightWall( sf::Vector2f( 50.0, 50.0 ), sf::Vector2f( 50.0, 5.0 ) );
    Wall topWall( sf::Vector2f( 5.0, 5.0 ), sf::Vector2f( 50.0, 5.0 ) );

    Player player( sf::Vector2f( 25.0, 25.0 ) );

    while ( window.isOpen() )
    {
        sf::Event event;

        while ( window.pollEvent( event ) )
        {
            switch ( event.type )
            {
                case sf::Event::Closed:
                    window.close();
                    break;

                case sf::Event::KeyPressed:
                {
                    if ( sf::Keyboard::isKeyPressed( sf::Keyboard::D ) )
                        player.moveRight();
                    else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::A ) )
                        player.moveLeft();
                    else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::W ) )
                        player.moveForward();
                    else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::S ) )
                        player.moveBackward();
                    else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::Q ) )
                        player.changeAngleBy( -0.1 );
                    else if ( sf::Keyboard::isKeyPressed( sf::Keyboard::E ) )
                        player.changeAngleBy( 0.1 );
                    
                    break;
                }

                default:
                    break;
            }
        }

        window.clear();
        window.draw( player );
        window.draw( leftWall );
        window.draw( rightWall );
        window.draw( topWall );

        window.draw( Wall::perspectiveTransform( leftWall.relativeTransform( player ) ) );
        window.draw( Wall::perspectiveTransform( rightWall.relativeTransform( player ) ) );
        window.draw( Wall::perspectiveTransform( topWall.relativeTransform( player ) ) );

        window.display();
    }

    return 0;
}
